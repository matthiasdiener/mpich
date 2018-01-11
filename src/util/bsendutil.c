/*
 *  $Id: bsendutil.c,v 1.8 1996/06/07 15:12:34 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 *
 * The handling of nonblocking bsend operations needs some work.  Currently,
 * There is a single request for a nonblocking bsend operation, and this can
 * cause problems when we try to complete a nonblocking bsend operation, becase
 * both we and the user may have a copy of the same request.  
 *
 * The solution to this is a little complicated.  Note that the MPI standard
 * requires that you can free an active request (just like the other MPI
 * objects, freeing an object just decrements its reference count; anything
 * that makes an object "active" increments its reference count).  
 * So, one solution is to implement this reference count, and then make
 * use of it here (so that MPI_TEST will execute the Free and set the 
 * pointer to NULL, but the actual free won't happen until the ref count is
 * set to zero).
 *
 * But to really do this, we need have some way to complete a nonblocking
 * operation even though the user will never again call it with a WAIT
 * or TEST call.  
 *
 * As a short term fix, we ONLY call MPI_TEST in this code for blocking
 * BSENDs; this is safe, because the ONLY copy of the request is here.
 * Thus, the test on whether to check a request includes a check on the
 * blocking nature.  Note also that the routine called to free a request
 * calls a special routine (MPIR_BufferFreeReq), so we can keep the
 * information here properly updated.
 *
 * Another approach, which I discussed with Hubertus, would be to alloc a
 * new request, have the buffer point at that, and copy all of the relavent
 * details into the given buffer.
 *
 * The "best" thing to do depends on how you interpret the various flavors
 * of buffered send:
 *    Method 1.  Bsend, Ibsend, and Bsend_init/Start all copy the data
 *    into a buffer; when the data is copied, the routines return.  In this
 *    case, both Ibsend and Bsend_init/Start should indicate that the 
 *    send has completed, since the data INPUT to these routines has 
 *    been copied and my now be re-used.  (There is, thank goodness, no
 *    Ibs(ync)send).  Note that in this case, the user's request and the
 *    internal request are VERY different.
 *    
 *    Method 2.  Ibsend and Bsend_init would not complete coping data into
 *    the buffer until a later time.  This may be intended for systems with
 *    special move engines that operate asynchronously; some mechanism
 *    would be required to determine completion.  
 *
 * My choice is to copy the request and mark the "users" request as completed
 * when the data has been moved.
 */


#ifdef MPI_ADI2
#include "bsendutil2.c"
#else

/* #define DEBUG_BSEND */

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */ 
static int DebugBsend = 0;
#define DEBUG_PRINT(str) PRINTF( "%s\n", str );
#else
#define DEBUG_PRINT(str) 
#endif                 /* #DEBUG_BSEND_END# */

#include "mpiimpl.h"
#include "mpisys.h"
#ifndef MEMCPY
#define MEMCPY(a,b,n) memcpy(a,b,n)
#endif

/* 
   This file contains the code for managing the "Buffered" sends (with 
   a user-provided buffer).  This uses the simple buffer scheme described 
   in the MPI standard.

   Because the data in this list is sensitive, and because we could easily
   overwrite the data if we are not careful, I've added "Cookies" around the
   data.
 */
#define BSEND_HEAD_COOKIE 0xfea7600d
#define BSEND_TAIL_COOKIE 0xcadd5ac9
typedef struct _bsenddata {
    long              HeadCookie;
    struct _bsenddata *next, *prev;
    MPI_Request       req;             /* This is the actual request that
					  is used to send the message; 
					  note that this is a POINTER to the
					  appropriate structure.  It is
					  ALSO not the user's request,
					  in the case that a nonblocking
					  buffered send is used. */
    int               len;
    /* Information about the data to send, for persistent, nonblocking
       requests */
    int               count;
    MPI_Datatype      datatype;
    void              *buf;
    long              TailCookie;
    } BSendData;

/* If "req" is null, the block is not in use */
static BSendData *Bsend = 0;
static int BsendSize = 0;

int MPIR_TestBufferPtr ANSI_ARGS(( BSendData *));
BSendData *MPIR_MergeBlock ANSI_ARGS(( BSendData *));

/*
   MPIR_SetBuffer - Set the buffer area for the buffered sends, and 
   initialize the internal data structures
 */
int MPIR_SetBuffer( bufp, size )
void *bufp;
int  size;
{
BSendData *p;

DEBUG_PRINT("Starting MPIR_SetBuffer")
if (size < sizeof(BSendData)) 
    return MPI_ERR_OTHER;  /* Buffer too small */
if (Bsend)
    return MPI_ERR_BUFFER_EXISTS;
p	  = (BSendData *)bufp;
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
if (DebugBsend) 
    FPRINTF( stderr, "Initializing buffer to %d bytes at %x\n", size, 
	     (MPI_Aint) p );
#endif                 /* #DEBUG_BSEND_END# */
p->next	      = 0;
p->prev	      = 0;
p->req	      = 0;
p->len	      = size - sizeof(BSendData);
p->HeadCookie = BSEND_HEAD_COOKIE;
p->TailCookie = BSEND_TAIL_COOKIE;
BsendSize     = size;
Bsend	      = p;

DEBUG_PRINT("Exiting MPIR_SetBuffer" )
return MPI_SUCCESS;
}

/*
    Tests that a buffer area has not been corrupted by checking sentinals
    at the head and tail of a buffer area.
 */
int MPIR_TestBufferPtr( b )
BSendData *b;
{
return (b->HeadCookie != BSEND_HEAD_COOKIE ||
	b->TailCookie != BSEND_TAIL_COOKIE);
}

/* 
   Free a buffer (MPI_BUFFER_DETACH).  Note that this will wait to
   complete any pending operations.

   This routine is called by MPI_Finalize to make sure than any pending
   operations are completed.

   When called, it returns the current buffer and size in its arguments
   (both are output).
 */
void MPIR_FreeBuffer( buf, size )
void **buf;
int  *size;
{
BSendData *p = Bsend;
MPI_Status status;

DEBUG_PRINT("Entering MPIR_FreeBuffer")
/* If we are using the buffer, we must first wait on all pending messages */
while (p) {
    if (MPIR_TestBufferPtr(p)) {
	/* Error in pointer */
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		   "Error in BSEND data, corruption detected in FreeBuffer" );
	}
    if (p->req) {
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	if (DebugBsend) 
	    FPRINTF( stderr, 
		    "Waiting for release of buffer at %x with request %x\n",
		     (MPI_Aint) p, (MPI_Aint)p->req );
#endif                 /* #DEBUG_BSEND_END# */
	MPI_Wait( &p->req, &status );
	}
    p = p->next;
    }
*buf	  = (void *)Bsend;
*size	  = BsendSize;
Bsend	  = 0;
BsendSize = 0;
DEBUG_PRINT("Exiting MPIR_FreeBuffer")
}

/* 
   This is an internal routine for merging bsend buffer blocks.
   Merge b with any previous or next empty blocks.  Return the block to use
   next 
*/
BSendData *MPIR_MergeBlock( b )
BSendData *b;
{
BSendData *tp, *nextb;

DEBUG_PRINT("Entering MPIR_MergeBlock" )
nextb = b;
tp    = b->prev;
if (tp && MPIR_TestBufferPtr(tp)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in MergeBlock" );
    }

if (tp && tp->req == MPI_REQUEST_NULL) {
    /* Merge with previous block */
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
    if (DebugBsend) 
	FPRINTF( stderr, "Merging block at %x with next block\n", 
		 (MPI_Aint)tp );
#endif                 /* #DEBUG_BSEND_END# */
    tp->next = b->next;
    if (b->next) b->next->prev = tp;
    tp->len += b->len + sizeof(BSendData);
    b	     = tp;
    nextb    = b;
    }
tp = b->next;
if (tp && MPIR_TestBufferPtr(tp)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in MergeBlock" );
    }
if (tp && tp->req == MPI_REQUEST_NULL) {
    /* Merge with next block */
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
    if (DebugBsend) 
	FPRINTF( stderr, "Merging block at %x with previous block at %x\n", 
		 (MPI_Aint)tp, (MPI_Aint)b );
#endif                 /* #DEBUG_BSEND_END# */
    b->next = tp->next;
    if (tp->next) tp->next->prev = b->prev;
    b->len += tp->len + sizeof(BSendData);
    }
DEBUG_PRINT("Exiting MPIR_MergeBlock")
return nextb;
}

/* 
   The input to this routine is a size (in bytes) and an already created
   MPI_Request; the output is a pointer to the allocated buffer space.
   It also holds all of the information needed to pack the data, in 
   the event that this is a persistent, non-blocking, buffered send (!).

   Note that this must be called ONLY after all other fields in the 
   incoming request are set.  This routine will modify the request
   by marking it as completed.
 */
int MPIR_GetBuffer( size, rq, buf, count, datatype, bufp )
int          size;
MPI_Request  rq;
void         *buf;
int          count;
MPI_Datatype datatype;
void         **bufp;
{
BSendData  *b, *new;
int        flag;
MPI_Status status;

#ifdef MPI_ADI2
printf( "Not done - GetBuffer (Bsend)\n" );
  MPI_Abort( MPI_COMM_WORLD, 1 );
#else
DEBUG_PRINT("Entering MPIR_GetBuffer")
/* Round size to a multiple of 8 */
if (size & 0x7) size += (8 - (size & 0x7));
b = Bsend;
while (b) {
    if (MPIR_TestBufferPtr(b)) {
	/* Error in pointer */
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		   "Error in BSEND data, corruption detected in GetBuffer" );
	}
    /* Note that since the request in the bsend data is private, we can
       always execute this test */
    if (b->req != MPI_REQUEST_NULL && b->req->shandle.active) {
	/* Test for completion; merge if necessary.  If the request
	   is not active, we don't do the test. */
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	if (DebugBsend)
	    FPRINTF( stderr, "Testing for completion of block at %x\n",
		     (MPI_Aint)b );
#endif                 /* #DEBUG_BSEND_END# */
	MPI_Test( &b->req, &flag, &status );
	/* If completed and not persistant, remove */
	if (flag && !b->req) {
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	    if (DebugBsend)
		FPRINTF( stderr, "Found completed bsend\n" );
#endif                 /* #DEBUG_BSEND_END# */
	    /* Done; merge the blocks and test again */
	    b = MPIR_MergeBlock( b );
	    continue;
	    }
	}
    if (b->req == MPI_REQUEST_NULL) {
	/* Try to merge with surrounding blocks */
	b = MPIR_MergeBlock( b );
	}
    if (b->req == MPI_REQUEST_NULL && b->len >= size) {
	/* Split the block if there is enough room */
	if (b->len > size + sizeof(BSendData) + 8) {
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	    if (DebugBsend)
		FPRINTF( stderr, 
			"Found large block of size %d (need %d) at %x\n",
			b->len, size, (MPI_Aint)b );
#endif                 /* #DEBUG_BSEND_END# */
	    new			       = (BSendData *)(((char *)b) + 
						 sizeof(BSendData) + size);
	    new->next		       = b->next;
	    if (b->next) b->next->prev = new;
	    new->prev		       = b;
	    b->next		       = new;
	    new->len		       = b->len - size - sizeof(BSendData);
	    new->req		       = MPI_REQUEST_NULL;
	    new->HeadCookie	       = BSEND_HEAD_COOKIE;
	    new->TailCookie	       = BSEND_TAIL_COOKIE;
	    b->len		       = size;
	    }
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	if (DebugBsend)
	    FPRINTF( stderr, 
                 "Creating bsend block at %x of size %d\n", 
		    (MPI_Aint)b, size );
#endif                 /* #DEBUG_BSEND_END# */
	*bufp			 = (void *)(b+1);
	/* Copy the request to the local area */
	b->req			 = (MPI_Request) MPIR_SBalloc( MPIR_shandles );
	if (!b->req) {
	    return MPI_ERR_EXHAUSTED;
	}
	MEMCPY( b->req, rq, sizeof(MPIR_SHANDLE) );
	/* Save the buffer address */
	b->req->shandle.bufadd   = *bufp;

	/* These may no longer be needed */
	b->buf			 = buf;
	b->count		 = count;
	b->datatype		 = datatype;
	/* These may no longer be needed (to here) */

	/* Mark in the request (user's) where the corresponding bsend 
	   area is */
	rq->shandle.bsend	 = (void *)b;
	/* Also remember in the bsend request */
	b->req->shandle.bsend	 = (void *)b;
	/* Change the datatype and size, since we're going to use MPI_PACK */
	b->req->shandle.datatype = MPI_PACKED;
	b->req->shandle.count	 = size;
	/* The mode of the actual operation is just SEND */
	b->req->shandle.mode     = MPIR_MODE_STANDARD;
	DEBUG_PRINT("Exiting MPIR_GetBuffer")
	return MPI_SUCCESS;
	}
    b = b->next;
    }

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
FPRINTF( stdout, "Could not find %d bytes in buffer\n", size );
MPIR_BufferPrint();
#endif                 /* #DEBUG_BSEND_END# */
DEBUG_PRINT("Exiting MPIR_GetBuffer")
#endif
return MPI_ERR_USER_BUFFER_EXHAUSTED;
}

/* 
   This routine actually transfers the data from the users buffer to the
   internal buffer.  A bsend area must already exist for it, and be
   marked by bine set in the rq->bsend field (see the MPIR_SHANDLE structure).
 */
void MPIR_PrepareBuffer( rq )
MPIR_SHANDLE *rq;
{
BSendData *b;
int       outcount, position = 0;
MPIR_SHANDLE *brq;

#ifdef MPI_ADI2
printf( "Not done - PrepareBuffer (Bsend)\n" );
  MPI_Abort( MPI_COMM_WORLD, 1 );
#else
DEBUG_PRINT("Entering MPIR_PrepareBuffer")
b = (BSendData *)(rq->bsend);
if (!b) {
    MPIR_ERROR( rq->comm, MPI_ERR_OTHER, "Error in BSEND data" );
    return;
    }
if (MPIR_TestBufferPtr(b)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in PrepareBuffer" );
    }
/* This really should be the same as rq now... */
brq = (MPIR_SHANDLE *)b->req;
if (rq != brq) {
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER,
	       "Error in BSEND data; requests do not match" );
    }
outcount = b->len;
MPI_Pack( b->buf, b->count, b->datatype, brq->bufadd, outcount, &position, 
	  brq->comm );
brq->dev_shandle.start = brq->bufadd;
/* Make sure that the msgrep is correct */
#ifdef MPID_HAS_HETERO
    if ((MPID_IS_HETERO == 1) && 
	MPIR_Comm_needs_conversion(rq->comm))
	brq->msgrep = MPIR_MSGREP_XDR;
    else 
	brq->msgrep = MPIR_MSGREP_SENDER;
#endif

/* The number of bytes actually taken is returned in position */
brq->count			 = position;
brq->dev_shandle.bytes_as_contig = position;
if (MPIR_TestBufferPtr(b)) {
    /* Error in pointer after we've packed into it */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
     "Error in BSEND data, corruption detected at end of PrepareBuffer" );
    }
if (b->next && MPIR_TestBufferPtr(b->next)) {
    /* Error in pointer after we've packed into it */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
     "Error in BSEND data, corruption detected at data end of PrepareBuffer" );
    }
DEBUG_PRINT("Exiting MPIR_PrepareBuffer")
#endif
}

/* 
   Set the persistant flag for a request
 */
void MPIR_BsendPersistent( request, flag )
MPI_Request request;
int         flag;
{
    MPIR_SHANDLE *req;
    BSendData *b;

#ifdef MPI_ADI2
    b   = (BSendData *)request->shandle.bsend;
    if (flag) 
	b->req->handle_type = MPIR_PERSISTENT_SEND;
    else
	b->req->handle_type = MPIR_SEND;
#else
    b   = (BSendData *)request->shandle.bsend;
    req = (MPIR_SHANDLE *)b->req;
    req->persistent = flag;
#endif
}

/*
 * Perform the actual send, putting the data into the buffer
 */
int MPIR_DoBufferSend( shandle )
MPIR_SHANDLE *shandle;
{
    MPIR_SHANDLE *req;
    BSendData *b;
/* Find the local request */

#ifdef MPI_ADI2
    printf( "Not done - DoBufferSend (Bsend)\n" );
    MPI_Abort( MPI_COMM_WORLD, 1 );
#else
    b = shandle->bsend;
    if (MPIR_TestBufferPtr(b)) {
	/* Error in pointer after we've packed into it */
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		    "Error in BSEND area while trying to start communication" );
    }
    req = (MPIR_SHANDLE *)b->req;

/* Need to prepare the buffer before sending.  This replaces MPIR_Send_setup
 */
/* if (mpi_errno = MPIR_Send_setup( &req )) return mpi_errno; */
    req->active       = 1;
    MPIR_PrepareBuffer( req );

    MPID_Post_send( req->comm->ADIctx, req );

/* At this point, the shandle is done (but it is active) */
    shandle->active = 1;
    MPID_Set_completed( shandle->comm->ADIctx, (MPI_Request)shandle );
#endif
    return MPI_SUCCESS;
}

/* 
   Mark a request as free in the Buffer code 
   This is called only in MPI_Request_free (commreq_free.c)

   Note that we never want a USER call to free an INTERNAL buffer request.
   We do this by marking the request used by these routines as a regular
   send (MPIR_SEND) which it is.

   We may actually not need this routine, since we handle the case internally
   in the get/merge code.
 */
void MPIR_BufferFreeReq( rq )
MPIR_SHANDLE *rq;
{
BSendData *b;

DEBUG_PRINT("Entering MPIR_BufferFreeReq")
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
if (DebugBsend)
    FPRINTF( stderr, 
	    "Nulling Bsend request at %x\n", (MPI_Aint) b );
#endif                 /* #DEBUG_BSEND_END# */
if (!rq->bsend) return;
b      = (BSendData *)(rq->bsend);
if (MPIR_TestBufferPtr(b)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in FreeBuffer" );
    }
b->req = MPI_REQUEST_NULL;
DEBUG_PRINT("Exiting MPIR_BufferFreeReq")
}

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
/* 
 * This is a debugging routine 
 */
int MPIR_BufferPrint( )
{
BSendData *b;

FPRINTF( stdout, "Printing buffer arena\n" );
b = Bsend;
while (b) {
    if (MPIR_TestBufferPtr(b)) {
	/* Error in pointer */
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		   "Error in BSEND data, corruption detected in PrintBuffer" );
	}
    FPRINTF( stdout, "%x : len = %d, req = %x\n", (MPI_Aint)b, b->len, 
	    (MPI_Aint)(b->req) );
    b = b->next;
    }
FPRINTF( stdout, "End of printing buffer arena\n" );
return 0;
}
#endif                 /* #DEBUG_BSEND_END# */
#if defined(MPI_ADI2) && 0
/* This routine is called by MPI_Start to start an persistent bsend */
void MPIR_IbsendDatatype( comm, buf, count, datatype, src_lrank, tag, 
			  context_id, dest_grank, request, error_code )
MPI_Comm     comm;
MPI_Datatype datatype;
void         *buf;
int          count, src_lrank, tag, context_id, dest_grank, *error_code;
MPI_Request  request;
{
    /* The steps are */
    /* Pack data as necessary into buffer */
    /* init request */
    /* use ISendContig to send the message */
MPIR_DoBufferSend( request );
}
#endif

#endif
