/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


/* #define DEBUG(a) {a}  */
#define DEBUG(a)

/*
 * Routines for managing message queues in the device implementation.
 * Some devices provide their own queue management routines, and
 * do not need these.
 */

#include "mpid.h"
#include <stdio.h>
#include "sbcnst2.h"
/* Needed to allocate MPI_Requests */
#include "reqalloc.h"

/* Need to fix this... */
#ifdef FOO
#define MPID_SBalloc(a)  MALLOC(sizeof(MPID_QEL))
#define MPID_SBfree(a,b) FREE(b)
#endif
/* Used for SBcnst allocation; don't export */
static void *MPID_qels = 0;


/* #include "mpisys.h" */
#include "queue.h"

MPID_QHDR MPID_recvs;

/*
   Queueing unexpected messages and searching for them requires particular
   care because of the three cases:
   tag              source            Ordering
   xxx              xxx               Earliest message in delivery order
   MPI_ANY_TAG      xxx               Earliest with given source, not 
                                      lowest in tag value
   xxx              MPI_ANY_SOURCE    Earliest with given tag, not lowest
                                      in source.

   Note that only the first case is explicitly required by the MPI spec.
   However, there is a requirement on progress that requires SOME mechanism;
   the one here is easy to articulate and matches what many users expect.
   
   There are many comprimises that must be considered when deciding how to
   provide for these different criteria.  

   The code here takes the simple view that the number of unexpected messages
   is likely to be small, so it just keeps things in arrival order and
   searches for the first matching message. It might be nicer to have separate
   queues assocaited with each communicator. Hashing based on tag/source would
   also be possible, however it is unlikely to improve on the simple queue
   because the simple queue is still required, and the hash always adds
   overhead while only saving under some circumstances.

   The current system does a linear search through the entire list, and 
   thus will always give the earliest in delivery order AS RECEIVED BY THE
   ADI.  We've had trouble with message-passing systems that the ADI is
   using not providing a fair delivers (starving some sources); this 
   should be fixed by the ADI or underlying message-passing system rather
   than by this level of MPI routine.  
 */

void MPID_Dump_queues()
{
    MPID_Dump_queue( &MPID_recvs );
}
static int DebugFlag = 0;
void MPID_Dump_queue(header)
MPID_QHDR *header;
{
    MPID_QEL *p;

    if (!header) return;

    MPID_THREAD_DS_LOCK(header)
    p = header->unexpected.first;
    if (p) {
	fprintf( stdout, "[%d] Unexpected queue:\n", 
		 MPID_MyWorldRank );
    }
    while (p) {
	if (DebugFlag) {
	    fprintf( stdout, 
		 "[%d] %lx context_id = %d, tag = %d(%x), src = %d(%x)\n",
		 MPID_MyWorldRank, (MPI_Aint)p, 
		 p->context_id, p->tag, p->tagmask, p->lsrc, p->srcmask );
	}
	else {
	    /* This is a "users" form of the output */
	    fprintf( stdout, 
		 "[%d] context_id = %d, tag = %d, src = %d\n",
		 MPID_MyWorldRank, p->context_id, p->tag, p->lsrc );
	}
	p = p->next;
	}
    p = header->posted.first;
    if (p) {
	fprintf( stdout, "[%d] Posted receive queue:\n", MPID_MyWorldRank );
    }
    while (p) {
	if (DebugFlag) {
	    fprintf( stdout, 
		 "[%d] %lx context_id = %d, tag = %d(%x), src = %d(%x)\n",
		 MPID_MyWorldRank, (MPI_Aint)p,
		 p->context_id, p->tag, p->tagmask, p->lsrc, p->srcmask );
	}
	else {
	    fprintf( stdout, 
		 "[%d] context_id = %d, tag = ", MPID_MyWorldRank,
		 p->context_id );
	    if (p->tagmask == -1) fprintf( stdout, "%d", p->tag );
	    else                  fprintf( stdout, "MPI_ANY_TAG" );
	    fprintf( stdout, ", src = " );
	    if (p->srcmask == -1) fprintf( stdout, "%d\n", p->lsrc );
	    else                  fprintf( stdout, "MPI_ANY_SOURCE\n" );
	}
	p = p->next;
	}
    MPID_THREAD_DS_UNLOCK(p)
}


int MPID_Enqueue( header, src_lrank, tag, context_id, rhandle )
MPID_QUEUE    *header;
int           src_lrank, tag, context_id;
MPIR_RHANDLE  *rhandle;
{
    MPID_QEL *p;

    DEBUG(printf( "[%d] Before enqueing...\n", MPID_MyWorldRank ));
    DEBUG(MPID_Dump_queue(&MPID_recvs));
    p           = (MPID_QEL *) MPID_SBalloc( MPID_qels );
    if (!p) return MPI_ERR_EXHAUSTED;
    p->ptr      = rhandle;
    /* Store the selection criteria is an easy-to-get place */
    p->context_id = context_id;
    if (tag == MPI_ANY_TAG) {
	p->tagmask = 0;       /* No need to store the tag */
	}
    else {
	p->tag     = tag; 
	p->tagmask = ~0;
	}
    if (src_lrank == MPI_ANY_SOURCE) {
	p->srcmask = 0;       /* No need to store the source */
	}
    else {
	p->lsrc    = src_lrank;
	p->srcmask = ~0;
	}

    DEBUG(
    printf( "[%d] Enqueing msg with (%d,%d,%d) in elm at %lx\n", 
	    MPID_MyWorldRank, 
	    p->context_id, (p->tag & p->tagmask), (p->lsrc & p->srcmask), 
	    (MPI_Aint)p ));

    /* Insert at the tail of the queue, nice and simple ! */
    *(header->lastp)= p;
    p->next         = 0;
    header->lastp   = &p->next;

    DEBUG(MPID_Dump_queue(&MPID_recvs));
    return MPI_SUCCESS;
}

/*
 * Remove the given request from the queue (for MPID_Cancel)
 *
 * Returns 0 on success, MPI_ERR_INTERN on not found.
 */
int MPID_Dequeue( header, rhandle )
MPID_QUEUE   *header;
MPIR_RHANDLE *rhandle;
{
    MPID_QEL **pp;
    MPID_QEL *p;

    /* Look for the one we need */
    for (pp = &(header->first); 
	 (p = *pp) != 0; 
	 pp = &p->next) 
    {
	if (p->ptr == rhandle )
	    break;
    }

    if (p == NULL)
	return MPI_ERR_INTERN;		/* It's not there. Oops */
    else
    {					/* Remove from the Q and delete */
	if ((*pp = p->next) == 0)		
	    header->lastp = pp;		/* Fix the tail ptr */
	
	MPID_SBfree( MPID_qels, p );	/* free queue element */
    }	    
    return MPI_SUCCESS;
}

/* 
   Search posted_receive queue for message matching criteria 

   A flag of 1 causes a successful search to delete the element found 

   handleptr is set to non-null if an entry is found.
 */
int MPID_Search_posted_queue( src, tag, context_id, flag, handleptr )
register int          src, tag;
register int          context_id;
MPIR_RHANDLE          **handleptr;
int                   flag;
{
  /* Eventually, we'll want to separate this into the three cases
     described above.  We may also want different queues for each
     context_id 
     The tests may become
     
     if (tag == MPI_ANY_TAG) 
     if (tag == MPI_ANY_SOURCE) 
     take first message with matching context
     else
     search for context_id, source
     else if (SOURCE == MPI_ANY_SOURCE)
     search for context_id, tag
     else
     search for context_id, tag, source
  */
    MPID_QUEUE *queue = &MPID_recvs.posted;
    MPID_QEL   **pp;
    MPID_QEL   *p;

    /* Look for the one we need.
     * Note that by doing the tests using 'xor' and 'and' we don't need
     * to worry about the value in tag when tagmask is zero. (i.e. a wildcard
     * receive). This saves us from ever bothering to store a tag in that
     * case.
     * Remember that xor (^) is a bit wise not equal function.
     */
    for (pp = &(queue->first); 
	 (p = *pp) != 0; 
	 pp = &p->next)
    {
	if ( context_id        == p->context_id &&
	     ((tag ^ p->tag) & p->tagmask) == 0  &&
	     ((src ^ p->lsrc)& p->srcmask) == 0 )
	{
	    *handleptr = p->ptr;
	    if (flag)
	    {				/* Take it out */
		if ((*pp = p->next) == 0)		
		    queue->lastp = pp;	/* Fix the tail ptr */
		
		MPID_SBfree( MPID_qels, p); /* free queue element */
	    }
	    return MPI_SUCCESS;
	}
    }
    *handleptr = NULL;
    return MPI_SUCCESS;
}

#ifdef FOO
/* 
   Search posted_receive queue for a particular request (for implementing
   cancel)

   found is set to 1 for a successful search.  The element is dequeued
   if found.
 */
int MPID_Search_posted_for_request( handle, found )
MPIR_RHANDLE          *handle;
int                   *found;
{
    MPID_QUEUE   *queue = &MPID_recvs.posted;
    MPID_QEL     *p;

    p      = queue->first;
    while (p)
    {
	if (p->ptr == handle) {
	    *found = 1;
	    /* Delete and free the queue element */
	    if ( p->next != NULL )
		p->next->prev = p->prev;
	    else
		queue->last  = p->prev;
	    
	    if ( p->prev != NULL )
		p->prev->next = p->next;
	    else
		queue->first = p->next;
	    MPID_SBfree( MPID_qels, p);	/* free queue element */
	    return MPI_SUCCESS;
	}
	p   = p->next;
    }
    *found = 0;
    return MPI_SUCCESS;

    /* From the Dequeue code */
    MPID_QEL **pp;
    MPID_QEL *p;

    /* Look for the one we need */
    for (pp = &(header->first); 
	 (p = *pp) != 0; 
	 pp = &p->next)
    {
	if (p->ptr == rhandle )
	break;
    }

  if (p == NULL)
    return MPI_ERR_INTERN;		/* It's not there. Oops */
  else
    {					/* Remove from the Q and delete */
      if ((*pp = p->next) == 0)		
	header->lastp = pp;		/* Fix the tail ptr */
	
      MPID_SBfree( MPID_qels, p );	/* free queue element */
    }	    
  return MPI_SUCCESS;

}
#endif

/* search unexpected_recv queue for message matching criteria

   A flag of 1 causes a successful search to delete the element found 

   handleptr is non-null if an element is found
 */
int MPID_Search_unexpected_queue( src, tag, context_id, flag, handleptr )
register int src, tag;
register int context_id;
int          flag;
MPIR_RHANDLE **handleptr;
{
  MPID_QUEUE   *queue = &MPID_recvs.unexpected;
  MPID_QEL     **pp;
  MPID_QEL      *p;
  int          tagmask, srcmask;

  tagmask = (tag == MPI_ANY_TAG)    ? 0 : ~0;
  srcmask = (src == MPI_ANY_SOURCE) ? 0 : ~0;

  DEBUG(printf( "[%d] searching for (%d,%d,%d) in unexpected queue\n", 
	       MPID_MyWorldRank, context_id, tag, src ));
  /* Look for the one we need */
  for (pp = &(queue->first); 
       (p = *pp) != 0; 
       pp = &p->next)
    {
      DEBUG(printf("[%d] in unexpected list, looking at (%d,%d,%d) at %lx\n",
		   MPID_MyWorldRank, 
		   p->context_id, (p->tag & tagmask), (p->lsrc & srcmask), 
		   (MPI_Aint)p ));
      if (context_id ==  p->context_id      &&
	  (((tag ^ p->tag)  & tagmask) == 0) &&
	  (((src ^ p->lsrc) & srcmask) == 0) )
	{
	  *handleptr = p->ptr;
	  if (flag)
	    {
	      if ((*pp = p->next) == 0)		
		queue->lastp = pp;	/* Fix the tail ptr */
	      MPID_SBfree( MPID_qels, p); /* free queue element */
	    }
	  return MPI_SUCCESS;
	}
    }
  *handleptr = 0;
  return MPI_SUCCESS;
}

/* called by device when a message arrives.  Returns 1 if there is posted
 * receive, 0 otherwise.
 *
 * This puts the responsibility for searching the unexpected queue and
 * posted-receive queues on the device.  If it is operating asynchronously
 * with the user code, the device must provide the necessary locking mechanism.
 */

void MPID_Msg_arrived( src, tag, context_id, dmpi_recv_handle, foundflag )
int          src, tag, *foundflag;
int          context_id;
MPIR_RHANDLE **dmpi_recv_handle;
{
    MPIR_RHANDLE *handleptr;

    MPID_THREAD_DS_LOCK(MPID_recv)
    MPID_Search_posted_queue( src, tag, context_id, 1, dmpi_recv_handle);
    if ( *dmpi_recv_handle )
    {
        MPID_THREAD_DS_UNLOCK(MPID_recvs)
	*foundflag = 1;	
	/* note this overwrites any wild-card values in the posted handle */
	handleptr         	= *dmpi_recv_handle;
	handleptr->s.MPI_SOURCE	= src;
	handleptr->s.MPI_TAG 	= tag;
	/* count is set in the put and get routines */
    }
    else
    {
	/* allocate handle and put in unexpected queue */
	*dmpi_recv_handle       = MPID_RecvAlloc();
	/* Note that we don't initialize the request here, because 
	   we're storing just the basic information on the request,
	   and all other fields will be set when the message is found.
	   However, for debugging purposes, we can initialize with 
	   non-zero values */
	handleptr         	= *dmpi_recv_handle;
	if (!handleptr) {
	    MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_INTERN,
		        "Could not dynamically allocate internal handle" );
	    }
#ifdef MPID_DEBUG_ALL
	memset( handleptr,0xfa, sizeof(MPIR_RHANDLE) );
#endif
	handleptr->s.MPI_SOURCE	= src;
	handleptr->s.MPI_TAG  	= tag;
	/* Note that we don't save the context id or set a datatype */
	handleptr->is_complete  = 0;
	
	MPID_Enqueue( &MPID_recvs.unexpected, src, tag, context_id, 
		      *dmpi_recv_handle );
	*foundflag = 0;
	MPID_THREAD_DS_UNLOCK(MPID_recvs)
    }
}

/*
 * This is a special thread-safe version to first check the unexpected queue
 * and then post the receive if a matching value is not first found in the
 * unexpected queue.  The entry is removed from the unexpected queue if
 * it is found.
 */
void MPID_Search_unexpected_queue_and_post( src_lrank, tag, context_id,  
					    request, rhandleptr )
int src_lrank, tag, context_id;
MPIR_RHANDLE *request, **rhandleptr;
{
    MPID_THREAD_DS_LOCK(&MPID_recvs)
    MPID_Search_unexpected_queue( src_lrank, tag, context_id, 1, rhandleptr );
    if (*rhandleptr) {
	MPID_THREAD_DS_UNLOCK(&MPID_recvs)
	return;
    }

    /* Add to the posted receive queue */
    MPID_Enqueue( &MPID_recvs.posted, src_lrank, tag, context_id, request );
    MPID_THREAD_DS_UNLOCK(&MPID_recvs)
}

#ifdef FOO
/*
   Let the device tell the API that a handle can be freed (this handle
   was generated by an unexpected receive and inserted by DMPI_msg_arrived.
 */
void MPID_Free_unexpected( dmpi_recv_handle  )
MPIR_RHANDLE      *dmpi_recv_handle;
{
/* ********** FIX ME ********* */
/* MPIR_SBfree( MPID_rhandles, dmpi_recv_handle ); */
}
#endif

void MPID_InitQueue()
{
    /* initialize queues */
    MPID_qels       = MPID_SBinit( sizeof( MPID_QEL ), 100, 100 );
    DEBUG(printf("[%d] About to setup message queues\n", MPID_MyWorldRank);)
    MPID_recvs.posted.first	= NULL;
    MPID_recvs.posted.lastp     = &MPID_recvs.posted.first;
    MPID_recvs.unexpected.first	= NULL;
    MPID_recvs.unexpected.lastp = &MPID_recvs.unexpected.first;

    MPID_THREAD_DS_LOCK_INIT(&MPID_recvs)
}
