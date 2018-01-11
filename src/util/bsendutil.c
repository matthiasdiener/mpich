/*
 *  $Id: bsendutil.c,v 1.2 1994/10/28 19:19:50 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: bsendutil.c,v 1.2 1994/10/28 19:19:50 gropp Exp $";
#endif /* lint */

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
static int DebugBsend = 0;
#endif                 /* #DEBUG_BSEND_END# */

#include "mpiimpl.h"

/* 
   This file contains the code for managing the "Buffered" sends (with 
   a user-provided buffer).  This uses the simple buffer scheme described 
   in the MPI standard.
 */

/*
   Because the data in this list is sensitive, and because we could easily
   overwrite the data if we are not careful, I've added "Cookies" around the
   data.
 */
#define BSEND_HEAD_COOKIE 0xfea7600d
#define BSEND_TAIL_COOKIE 0xcadd5ac9
typedef struct _bsenddata {
    long              HeadCookie;
    struct _bsenddata *next, *prev;
    MPI_Request       req;
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

int MPIR_SetBuffer( bufp, size )
void *bufp;
int  size;
{
BSendData *p;

if (size < sizeof(BSendData)) 
    return MPI_ERR_OTHER;  /* Buffer too small */
if (Bsend)
    return MPI_ERR_BUFFER_EXISTS;
p	  = (BSendData *)bufp;
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
if (DebugBsend) 
    fprintf( stderr, "Initializing buffer to %d bytes at %x\n", size, 
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

return MPI_SUCCESS;
}

int MPIR_TestBufferPtr( b )
BSendData *b;
{
return (b->HeadCookie != BSEND_HEAD_COOKIE ||
	b->TailCookie != BSEND_TAIL_COOKIE);
}

void MPIR_FreeBuffer( buf, size )
void **buf;
int  *size;
{
BSendData *p = Bsend;
MPI_Status status;

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
	    fprintf( stderr, "Waiting for release of buffer at %x\n",
		     (MPI_Aint) p );
#endif                 /* #DEBUG_BSEND_END# */
	MPI_Wait( &p->req, &status );
	}
    p = p->next;
    }
*buf	  = (void *)Bsend;
*size	  = BsendSize;
Bsend	  = 0;
BsendSize = 0;
}

/* Merge b with any previous or next empty blocks.  Return the block to use
   next */
BSendData *MPIR_MergeBlock( b )
BSendData *b;
{
BSendData *tp, *nextb;

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
	fprintf( stderr, "Merging block at %x with next block\n", 
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
	fprintf( stderr, "Merging block at %x with previous block\n", 
		 (MPI_Aint)tp );
#endif                 /* #DEBUG_BSEND_END# */
    b->next = tp->next;
    if (tp->next) tp->next->prev = b->prev;
    b->len += tp->len + sizeof(BSendData);
    }
return nextb;
}

/* 
   The input to this routine is a size (in bytes) and an already created
   MPI_Request; the output is a pointer to the allocated buffer space.
   It also holds all of the information needed to pack the data, in 
   the event that this is a persistent, non-blocking, buffered send (!).
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

/* Round size to a multiple of 8 */
if (size & 0x7) size += (8 - (size & 0x7));
b = Bsend;
while (b) {
    if (MPIR_TestBufferPtr(b)) {
	/* Error in pointer */
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		   "Error in BSEND data, corruption detected in GetBuffer" );
	}
    if (b->req != MPI_REQUEST_NULL) {
	/* Test for completion; merge if necessary */
	/* Should probably test ONLY if it was a blocking bsend; otherwise,
	   require the user to do the wait */
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	if (DebugBsend)
	    fprintf( stderr, "Testing for completion of block at %x\n",
		     (MPI_Aint)b );
#endif                 /* #DEBUG_BSEND_END# */
	MPI_Test( &b->req, &flag, &status );
	if (flag) {
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	    if (DebugBsend)
		fprintf( stderr, "Found completed bsend\n" );
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
		fprintf( stderr, 
			"Found large block of size %d (need %d) at %x\n",
			b->len, size, (MPI_Aint)b );
#endif                 /* #DEBUG_BSEND_END# */
	    new	      = (BSendData *)(((char *)b) + sizeof(BSendData) + size);
	    new->next = b->next;
	    if (b->next) b->next->prev = new;
	    new->prev = b;
	    b->next   = new;
	    new->len = b->len - size - sizeof(BSendData);
	    new->req = MPI_REQUEST_NULL;
	    new->HeadCookie = BSEND_HEAD_COOKIE;
	    new->TailCookie = BSEND_TAIL_COOKIE;
	    b->len   = size;
	    }
#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
	if (DebugBsend)
	    fprintf( stderr, 
                 "Creating bsend block at %x of size %d\n", 
		    (MPI_Aint)b, size );
#endif                 /* #DEBUG_BSEND_END# */
	*bufp		     = (void *)(b+1);
	b->req		     = rq;
	b->buf		     = buf;
	b->count	     = count;
	b->datatype	     = datatype;
	rq->shandle.bsend    = (void *)b;
	/* Change the datatype and size, since we're going to use MPI_PACK */
	rq->shandle.datatype = MPI_PACKED;
	rq->shandle.count    = size;
	return MPI_SUCCESS;
	}
    b = b->next;
    }

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
MPIR_BufferPrint();
#endif                 /* #DEBUG_BSEND_END# */
return MPI_ERR_BUFFER | MPIR_ERR_USER_BUFFER_EXHAUSTED;
}

/* 
   This routine actually transfers the data from the users buffer to the
   internal buffer
 */
void MPIR_PrepareBuffer( rq )
MPIR_SHANDLE *rq;
{
BSendData *b;
int       outcount, position = 0;

b = (BSendData *)(rq->bsend);
if (!b) 
    MPIR_ERROR( rq->comm, MPI_ERR_OTHER, "Error in BSEND data" );
if (MPIR_TestBufferPtr(b)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in PrepareBuffer" );
    }
outcount = b->len;
MPI_Pack( b->buf, b->count, b->datatype, rq->bufadd, outcount, &position, 
	 rq->comm );
/* The number of bytes actually taken is returned in position */
rq->count			= position;
rq->dev_shandle.bytes_as_contig	= position;
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
}

/* Mark a request as free in the Buffer code */
void MPIR_BufferFreeReq( rq )
MPIR_SHANDLE *rq;
{
BSendData *b;

#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
if (DebugBsend)
    fprintf( stderr, 
	    "Nulling Bsend request at %x\n", (MPI_Aint) b );
#endif                 /* #DEBUG_BSEND_END# */
b      = (BSendData *)(rq->bsend);
if (MPIR_TestBufferPtr(b)) {
    /* Error in pointer */
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
	       "Error in BSEND data, corruption detected in FreeBuffer" );
    }
b->req = MPI_REQUEST_NULL;
}



#ifdef DEBUG_BSEND     /* #DEBUG_BSEND_START# */
int MPIR_BufferPrint( )
{
BSendData *b;

b = Bsend;
while (b) {
    if (MPIR_TestBufferPtr(b)) {
	/* Error in pointer */
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, 
		   "Error in BSEND data, corruption detected in PrintBuffer" );
	}
    fprintf( stdout, "%x : len = %d, req = %x\n", (MPI_Aint)b, b->len, 
	    (MPI_Aint)(b->req) );
    b = b->next;
    }
return 0;
}
#endif                 /* #DEBUG_BSEND_END# */
