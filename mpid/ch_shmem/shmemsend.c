/*
 *  $Id: chsend.c,v 1.34 1995/06/30 17:35:28 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chsend.c,v 1.34 1995/06/30 17:35:28 gropp Exp $";
#endif

#include "mpid.h"

/* 
   Still need to do:  clean up the post_short, post_long to look more like the
   code in chrecv.c .  Complicated slightly because the chrecv has already
   received part of the message, where here, the message header needs to
   be sent with, possibly, some of the data.

   There are many other strategies for IMPLEMENTING the ADI than the one
   shown here.  For example, a more deliberate packetizing strategy could 
   be used.  For systems with interrupt-driven receives, we could send 
   messages only in responce to a request.  If access to lower levels of
   the transport is available, then the protocols for transmitting a message
   can be customized to the ADI.

   Also to be done:  allow the ADI to dynamically allocate packets and
   store them in the (void *pkt) field in dev_shandle, allowing the use
   of non-blocking operations to send the message packets.  This is needed
   on some systems (like TMC-CMMD and IBM-MPL) that do not provide much
   internal buffering for the user.
 */

/* Here are some definitions to simplify debugging */
#include "mpiddebug.h"

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are outstanding                                      */
/***************************************************************************/
extern int MPID_n_pending;  /* Number of uncompleted split requests */

/***************************************************************************/

/* This routine is a hook for eventually allowing pre-initialized packets */
void MPID_SHMEM_Init_send_code()
{
}

/* Nonblocking packet allocation for sending? */

/* 
   This file includes the routines to handle the device part of a send
   for Chameleon

   As a reminder, the first element is the device handle, the second is
   the (basically opaque) mpi handle
 */

/* Send a short (single packet message) */
int MPID_SHMEM_post_send_short( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SEND_DECL(MPID_PKT_SHORT_T,pkt);
int              dest;

MPID_PKT_SEND_ALLOC(MPID_PKT_SHORT_T,pkt);
/* These references are ordered to match the order they appear in the 
   structure */
dest             = dmpi_send_handle->dest;
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_SHORT);
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
MPID_PKT_SEND_SET_HETERO(pkt,dmpi_send_handle->msgrep)

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

if (len > 0) {
    MEMCPY( MPID_PKT_SEND_GET(pkt,buffer), mpid_send_handle->start, len );
    DEBUG_PRINT_PKT_DATA("S Getting data from mpid->start",pkt)
    }
/* Always use a blocking send for short messages.
   (May fail with systems that do not provide adequate
   buffering.  These systems should switch to non-blocking sends)
 */
DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt)

/* In case the message is marked as non-blocking, indicate that we don't
   need to wait on it.  We may also want to use nonblocking operations
   to send the envelopes.... */
mpid_send_handle->sid = 0;
MPID_DRAIN_INCOMING_FOR_TINY(mpid_send_handle->is_non_blocking)
MPID_SENDCONTROL( mpid_send_handle, MPID_PKT_SEND_ADDR(pkt), 
		      len + sizeof(MPID_PKT_HEAD_T), dest );

DMPI_mark_send_completed( dmpi_send_handle );
MPID_PKT_SEND_FREE(pkt);

DEBUG_PRINT_MSG("S Sent message in a single packet")

return MPI_SUCCESS;
}

/* Long message */
#ifdef MPID_USE_GET
#elif !defined(MPID_USE_RNDV)
/* Message-passing or channel version of send long message */
int MPID_SHMEM_post_send_long_eager( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
char             *address;
int              pkt_len;
MPID_PKT_SEND_DECL(MPID_PKT_LONG_T,pkt);
int              dest;

MPID_PKT_SEND_ALLOC(MPID_PKT_LONG_T,pkt);
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_LONG);
pkt_len = sizeof(MPID_PKT_LONG_T); 
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
MPID_PKT_SEND_SET_HETERO(pkt,dmpi_send_handle->msgrep)
dest           = dmpi_send_handle->dest;

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

DEBUG_PRINT_LONG_MSG("S Sending extra-long message",pkt)

/* Send as packet only */
MPID_DRAIN_INCOMING_FOR_TINY(mpid_send_handle->is_non_blocking)
MPID_SENDCONTROL( mpid_send_handle, MPID_PKT_SEND_ADDR(pkt), pkt_len, dest );

/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
/* This may be non-blocking */
MPID_SendData( address, len, dest, mpid_send_handle )

MPID_PKT_SEND_FREE(pkt);
return MPI_SUCCESS;
}
#endif

#ifndef PI_NO_NSEND
MPID_SHMEM_Cmpl_send_nb( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;
if (mpid_send_handle->sid)  {
    /* Before we do the wait, try to clear all pending messages */
    (void)MPID_SHMEM_check_incoming( MPID_NOTBLOCKING );
    MPID_SHMEM_isend_wait( dmpi_send_handle );
    }
}
#endif

/*
   We should really:

   a) remove the sync_send code
   b) ALWAYS use the rndv code

   This will require calling the appropriate test and unexpected
   message routines.
 */
#ifndef MPID_USE_RNDV
int MPID_SHMEM_post_send_sync_short( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SEND_DECL(MPID_PKT_SHORT_SYNC_T,pkt);
int                   dest;

MPID_PKT_SEND_ALLOC(MPID_PKT_SHORT_SYNC_T,pkt);

/* These references are ordered to match the order they appear in the 
   structure */
dest             = dmpi_send_handle->dest;
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_SHORT_SYNC); 
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
MPID_PKT_SEND_SET_HETERO(pkt,dmpi_send_handle->msgrep)
MPID_PKT_SEND_SET(pkt,sync_id,
		  MPID_SHMEM_Get_Sync_Id( dmpi_send_handle, mpid_send_handle ));

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

if (len > 0) {
    MEMCPY( MPID_PKT_SEND_GET(pkt,buffer), mpid_send_handle->start, len );
    DEBUG_PRINT_PKT_DATA("",pkt)
    }
/* Always use a blocking send for short messages.
   (May fail with systems that do not provide adequate
   buffering.  These systems should switch to non-blocking sends, or use
   blocking if the message itself is in blocking mode.)
 */
DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt)

/* In case the message is marked as non-blocking, indicate that we don't
   need to wait on it */
mpid_send_handle->sid = 0;
MPID_SendControlBlock( MPID_PKT_SEND_ADDR(pkt), 
                  len + (sizeof(MPID_PKT_SHORT_SYNC_T)-MPID_PKT_MAX_DATA_SIZE),
		  dest );
dmpi_send_handle->completer = MPID_CMPL_SEND_SYNC;
DEBUG_PRINT_MSG("S Sent message in a single packet")

return MPI_SUCCESS;
}

/* Long message */
#ifdef MPID_USE_GET
#else
int MPID_SHMEM_post_send_sync_long_eager( dmpi_send_handle, mpid_send_handle, 
				       len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
char                 *address;
MPID_PKT_SEND_DECL(MPID_PKT_LONG_SYNC_T,pkt);
int                  dest;

MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_LONG_SYNC); 
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
MPID_PKT_SEND_SET_HETERO(pkt,dmpi_send_handle->msgrep)
MPID_PKT_SEND_SET(pkt,sync_id,
		  MPID_SHMEM_Get_Sync_Id( dmpi_send_handle, mpid_send_handle ));
dest           = dmpi_send_handle->dest;

DEBUG_PRINT_SEND_PKT("S Sending ",pkt)
DEBUG_PRINT_LONG_MSG("S Sending extra-long message",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

/* Send as packet only */
MPID_SendControlBlock( MPID_PKT_SEND_ADDR(pkt), 
		       sizeof(MPID_PKT_LONG_SYNC_T), dest );

/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
MPID_SendData( address, len, dest, mpid_send_handle )
dmpi_send_handle->completer = MPID_CMPL_SEND_SYNC;

return MPI_SUCCESS;
}
#endif

int MPID_SHMEM_Cmpl_send_sync( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;
#ifndef PI_NO_NSEND
if (mpid_send_handle->sid)  {
    /* Before we do the wait, try to clear all pending messages */
    (void)MPID_SHMEM_check_incoming( MPID_NOTBLOCKING );
    MPID_SHMEM_isend_wait( dmpi_send_handle );
    }
#endif

DEBUG_PRINT_MSG("S Entering complete send while loop")
while (!MPID_Test_handle(dmpi_send_handle)) {
    /* This waits for the completion of a synchronous send, since at
       this point, we've finished waiting for the PInsend to complete,
       or for a incremental get */
    (void)MPID_SHMEM_check_incoming( MPID_BLOCKING );
    }
DEBUG_PRINT_MSG("S Exiting complete send")
}
#else   /* non-rndv sync send */
int MPID_SHMEM_post_send_sync_long_eager( dmpi_send_handle, mpid_send_handle, 
				       len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_SHMEM_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, len );
}
int MPID_SHMEM_post_send_sync_short( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_SHMEM_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, len );
}
#endif  /* else of non-rndv sync send */

/*
   This sends the data.
   It takes advantage of being provided with the address of the user-buffer
   in the contiguous case.
 */
int MPID_SHMEM_post_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len, rc;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

DEBUG_PRINT_MSG("S Entering post send")

/* Eventually, we'd like to make this more dynamic */
if (actual_len > MPID_PKT_DATA_SIZE) 
#ifdef MPID_USE_GET
    rc = MPID_SHMEM_post_send_long_get( dmpi_send_handle, mpid_send_handle, 
				   actual_len );
#elif defined(MPID_USE_RNDV)
    rc = MPID_SHMEM_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, 
				      actual_len );
#else
    rc = MPID_SHMEM_post_send_long_eager( dmpi_send_handle, mpid_send_handle, 
				       actual_len );
#endif
else
    rc = MPID_SHMEM_post_send_short( dmpi_send_handle, mpid_send_handle, 
				    actual_len );

/* Poke the device in case there is data ... */
DEBUG_PRINT_MSG("S Draining incoming...")
MPID_DRAIN_INCOMING;
DEBUG_PRINT_MSG("S Exiting post send")

return rc;
}

int MPID_SHMEM_post_send_sync( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len, rc;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

if (actual_len > MPID_PKT_DATA_SIZE) 
#ifdef MPID_USE_RNDV
    rc = MPID_SHMEM_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, 
				      actual_len );
#elif defined(MPID_USE_GET)
    rc = MPID_SHMEM_post_send_sync_long_get( dmpi_send_handle, mpid_send_handle, 
					  actual_len );
#else
    rc = MPID_SHMEM_post_send_sync_long_eager( dmpi_send_handle, 
					    mpid_send_handle, actual_len );
#endif
else
    rc = MPID_SHMEM_post_send_sync_short( dmpi_send_handle, mpid_send_handle, 
					 actual_len );

/* Poke the device in case there is data ... */
MPID_DRAIN_INCOMING;

return rc;
}

int MPID_SHMEM_Blocking_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_LIMITED_BUFFERS
/* Force the use of non-blocking operations so that head-to-head operations
   can complete when there is an IRECV posted */
dmpi_send_handle->dev_shandle.is_non_blocking = 1;
MPID_SHMEM_post_send( dmpi_send_handle );
MPID_SHMEM_complete_send( dmpi_send_handle );
dmpi_send_handle->dev_shandle.is_non_blocking = 0;
#else
MPID_SHMEM_post_send( dmpi_send_handle );
MPID_SHMEM_complete_send( dmpi_send_handle );
#endif
return MPI_SUCCESS;
}

/*
  Chameleon gets no asynchronous notice that the message has been complete,
  so there is no asynchronous ref to DMPI_mark_send_completed.
 */
int MPID_SHMEM_isend_wait( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;

/* Wait on the message */
#ifndef PI_NO_NSEND
mpid_send_handle = &dmpi_send_handle->dev_shandle;
if (mpid_send_handle->sid) {
    /* We don't use non-blocking if the message is short enough... */
    /* We should probably ONLY do this in response to an explicit 
       note that the message has been received */
#ifdef MPID_LIMITED_BUFFERS
    /* We do this to keep us from blocking in a wait in the event that
       we must handle some incoming messages before we can execute the
       wait. */
    while (!MPID_TestSendTransfer(mpid_send_handle->sid))
	(void) MPID_SHMEM_check_incoming( MPID_NOTBLOCKING );
    /* Once we have it, the message is completed */
    mpid_send_handle->sid = 0;
#else
    MPID_WSendChannel( (void *)0, mpid_send_handle->bytes_as_contig, -1,
		      mpid_send_handle->sid );
    mpid_send_handle->sid = 0;
#endif
    }
#endif
if (dmpi_send_handle->mode != MPIR_MODE_SYNCHRONOUS) {
    DMPI_mark_send_completed( dmpi_send_handle );
    }

return MPI_SUCCESS;
}

/*
  We have to be careful here.  If the wait would block because a matching
  receive has not yet been posted on the destination end, we could deadlock.

  The "solution" here is to first clear any incoming messages.  This allows
  us to post a matching receive that this send is supposed to complete.
  This solution is not complete; there are race conditions that can still
  cause it to fail.  In addition, the current code to handle incoming messages
  may try to force the receive to complete first; this will cause some systems
  to deadlock.  We probably need to packetize to guarentee reliable 
  behavior, and allow for partial completion.  

  Deferred to a later implementation (or better systems!)
 */
int MPID_SHMEM_complete_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
/* Check to see if we need to complete the send. */
DEBUG_PRINT_MSG("S Entering complete send")

switch (dmpi_send_handle->completer) {
    case 0: 
         /* Message already completed */
         break;
#ifdef MPID_USE_RNDV
    case MPID_CMPL_SEND_RNDV:
	 MPID_SHMEM_Cmpl_send_rndv( dmpi_send_handle );
         break;
#endif
#ifdef MPID_USE_GET
    case MPID_CMPL_SEND_GET:
	 MPID_SHMEM_Cmpl_send_get( dmpi_send_handle );
         break;
#endif
#ifndef PI_NO_NSEND
    case MPID_CMPL_SEND_NB:
	 MPID_SHMEM_Cmpl_send_nb( dmpi_send_handle );
         break;
#endif
#ifndef MPID_USE_RNDV
    case MPID_CMPL_SEND_SYNC:
	 /* Also does non-blocking sync sends */
	 MPID_SHMEM_Cmpl_send_sync( dmpi_send_handle );
	 break;
#endif
    default:
	 fprintf( stdout, "[%d]* Unexpected send completion mode %d\n", 
	          MPID_MyWorldRank, dmpi_send_handle->completer );
	 break;
    }
DEBUG_PRINT_MSG("S Exiting complete send")

return MPI_SUCCESS;
}


/* 
   This routine tests for a send to be completed.  If non-blocking operations
   are used, it must check those operations...
 */
int MPID_SHMEM_Test_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_USE_RNDV
MPID_SHMEM_Test_send_rndv( dmpi_send_handle );
#endif
#ifndef PI_NO_NSEND
if (!MPID_Test_handle(dmpi_send_handle) &&
    dmpi_send_handle->dev_shandle.sid && 
    dmpi_send_handle->completer == MPID_CMPL_SEND_NB) {
    /* Note that if the test succeeds, the sid must be cleared; 
       otherwise we may attempt to wait on it later */
    if (MPID_TSendChannel( dmpi_send_handle->dev_shandle.sid )) {
	dmpi_send_handle->dev_shandle.sid = 0;
	return 1;
	}
    else 
	return 0;
    /* return MPID_TSendChannel( dmpi_send_handle->dev_shandle.sid ) ; */
    }
#endif
/* Need code for GET? */
return MPID_Test_handle(dmpi_send_handle);
}

/* 
   This routine makes sure that we complete all pending requests

   Note: We should make it illegal here to receive anything put
   things like DONE_GET and COMPLETE_SEND.
 */
int MPID_SHMEM_Complete_pending()
{
while (MPID_n_pending) {
    (void)MPID_SHMEM_check_incoming( MPID_BLOCKING );
    }
return MPI_SUCCESS;
}

