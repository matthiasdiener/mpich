/*
 *  $Id: chsend.c,v 1.30 1995/02/06 22:12:43 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chsend.c,v 1.30 1995/02/06 22:12:43 gropp Exp gropp $";
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
#ifdef MPID_DEBUG_ALL  
                       /* #DEBUG_START# */
#ifdef MEMCPY
#undef MEMCPY
#endif
#define MEMCPY(a,b,c)\
{if (MPID_DebugFlag) {\
    fprintf( MPID_DEBUG_FILE, \
	    "[%d]R About to copy to %d from %d (n=%d) (%s:%d)...\n", \
	    MPID_MyWorldRank, a, b, c, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE ); }\
memcpy( a, b, c );}

#define DEBUG_PRINT_SEND_PKT(msg,pkt)\
    {if (MPID_DebugFlag) {\
	fprintf( MPID_DEBUG_FILE,\
"[%d]%s of tag = %d, dest = %d, ctx = %d, len = %d, mode = ", \
	       MPID_MyWorldRank, msg, MPID_PKT_SEND_GET(pkt,tag), dest, \
	       MPID_PKT_SEND_GET(pkt,context_id), \
	       MPID_PKT_SEND_GET(pkt,len) );\
	MPID_Print_mode( MPID_DEBUG_FILE, MPID_PKT_SEND_ADDR(pkt) );\
	fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );\
	fflush( MPID_DEBUG_FILE );\
	}}

#define DEBUG_PRINT_MSG(msg)\
{if (MPID_DebugFlag) {\
    fprintf( MPID_DEBUG_FILE, "[%d]%s (%s:%d)\n", \
	    MPID_MyWorldRank, msg, __FILE__, __LINE__ );\
    fflush( MPID_DEBUG_FILE );}}
	    
                       /* #DEBUG_END# */
     
#else
#define DEBUG_PRINT_PKT(pkt)
#define DEBUG_PRINT_MSG(msg)
#define DEBUG_PRINT_SEND_PKT(msg,pkt)

#endif

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are outstanding                                      */
/***************************************************************************/
extern int MPID_n_pending;  /* Number of uncompleted split requests */

/***************************************************************************/
/* This variable controls debugging output                                 */
/***************************************************************************/
extern int MPID_DebugFlag;

/***************************************************************************/

/* This routine is a hook for eventually allowing pre-initialized packets */
void MPID_CH_Init_send_code()
{
}

/* 
   This file includes the routines to handle the device part of a send
   for Chameleon

   As a reminder, the first element is the device handle, the second is
   the (basically opaque) mpi handle
 */

/* Send a short (single packet message) */
int MPID_CH_post_send_short( dmpi_send_handle, mpid_send_handle, len ) 
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
#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    MPID_PKT_SEND_SET(pkt,has_xdr,MPID_MODE_XDR);
#endif                         /* #HETERO_END# */

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

if (len > 0) {
    MEMCPY( MPID_PKT_SEND_GET(pkt,buffer), mpid_send_handle->start, len );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (MPID_DebugFlag) {
	MPID_CH_Print_pkt_data( "S Getting data from mpid->start", 
			        MPID_PKT_SEND_GET(pkt,buffer), len );
	}
#endif                  /* #DEBUG_END# */
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
int MPID_CH_post_send_long( dmpi_send_handle, mpid_send_handle, len ) 
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
#ifdef MPID_HAS_HETERO      /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    MPID_PKT_SEND_SET(pkt,has_xdr,MPID_MODE_XDR);
#endif                      /* #HETERO_END# */
dest           = dmpi_send_handle->dest;

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    printf( 
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",
	   MPID_MyWorldRank, *(int *)mpid_send_handle->start, 
	   __FILE__, __LINE__ );
    printf( "[%d]S Sending extra-long message (%s:%d)...\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)MPID_PKT_SEND_ADDR(pkt) );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
/* Send as packet only */
MPID_DRAIN_INCOMING_FOR_TINY(mpid_send_handle->is_non_blocking)
MPID_SENDCONTROL( mpid_send_handle, MPID_PKT_SEND_ADDR(pkt), pkt_len, dest );

/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
#ifndef PI_NO_NSEND
if (mpid_send_handle->is_non_blocking) {
    MPID_ISendChannel( address, len, dest, mpid_send_handle->sid );
    dmpi_send_handle->completer = MPID_CMPL_SEND_NB;
    }
else 
#endif
    {
    mpid_send_handle->sid = 0;
    MPID_SendChannel( address, len, dest );
    DMPI_mark_send_completed( dmpi_send_handle );
    }
MPID_PKT_SEND_FREE(pkt);
return MPI_SUCCESS;
}
#endif

#ifndef PI_NO_NSEND
MPID_CH_Cmpl_send_nb( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;
if (mpid_send_handle->sid)  {
    /* Before we do the wait, try to clear all pending messages */
    (void)MPID_CH_check_incoming( MPID_NOTBLOCKING );
    MPID_CH_isend_wait( dmpi_send_handle );
    }
}
#endif

int MPID_CH_post_send_sync_short( dmpi_send_handle, mpid_send_handle, len ) 
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
#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    MPID_PKT_SEND_SET(pkt,has_xdr,MPID_MODE_XDR);
#endif                         /* #HETERO_END# */
MPID_PKT_SEND_SET(pkt,sync_id,
		  MPID_CH_Get_Sync_Id( dmpi_send_handle, mpid_send_handle ));

DEBUG_PRINT_SEND_PKT("S Sending",pkt)

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

if (len > 0) {
    MEMCPY( MPID_PKT_SEND_GET(pkt,buffer), mpid_send_handle->start, len );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (MPID_DebugFlag) {
	MPID_CH_Print_pkt_data( "S Getting data from mpid->start", 
			        MPID_PKT_SEND_GET(pkt,buffer), len );
	}
#endif                  /* #DEBUG_END# */
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
int MPID_CH_post_send_sync_long( dmpi_send_handle, mpid_send_handle, len ) 
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
#ifdef MPID_HAS_HETERO      /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    MPID_PKT_SEND_SET(pkt,has_xdr,MPID_MODE_XDR);
#endif                      /* #HETERO_END# */
MPID_PKT_SEND_SET(pkt,sync_id,
		  MPID_CH_Get_Sync_Id( dmpi_send_handle, mpid_send_handle ));
dest           = dmpi_send_handle->dest;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    DEBUG_PRINT_SEND_PKT("S Sending ",pkt)
    printf( 
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",
	   MPID_MyWorldRank, *(int *)&mpid_send_handle->start, 
	   __FILE__, __LINE__ );
    printf( "[%d]S Sending extra-long message (%s:%d)...\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)MPID_PKT_SEND_ADDR(pkt) );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */

MPID_PKT_PACK( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_HEAD_T), dest );

/* Send as packet only */
MPID_SendControlBlock( MPID_PKT_SEND_ADDR(pkt), 
		       sizeof(MPID_PKT_LONG_SYNC_T), dest );

/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
#ifndef PI_NO_NSEND
if (mpid_send_handle->is_non_blocking) {
    MPID_ISendChannel( address, len, dest, mpid_send_handle->sid );
    }
else 
#endif
    {
    mpid_send_handle->sid = 0;
    MPID_SendChannel( address, len, dest );
    }
dmpi_send_handle->completer = MPID_CMPL_SEND_SYNC;
return MPI_SUCCESS;
}
#endif

int MPID_CH_Cmpl_send_sync( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;
#ifndef PI_NO_NSEND
if (mpid_send_handle->sid)  {
    /* Before we do the wait, try to clear all pending messages */
    (void)MPID_CH_check_incoming( MPID_NOTBLOCKING );
    MPID_CH_isend_wait( dmpi_send_handle );
    }
#endif

DEBUG_PRINT_MSG("S Entering complete send while loop")
while (!MPID_Test_handle(dmpi_send_handle)) {
    /* This waits for the completion of a synchronous send, since at
       this point, we've finished waiting for the PInsend to complete,
       or for a incremental get */
    (void)MPID_CH_check_incoming( MPID_BLOCKING );
    }
DEBUG_PRINT_MSG("S Exiting complete send")
}

/*
   This sends the data.
   It takes advantage of being provided with the address of the user-buffer
   in the contiguous case.
 */
int MPID_CH_post_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len, rc;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

if (actual_len > MPID_PKT_DATA_SIZE) 
#ifdef MPID_USE_GET
    rc = MPID_CH_post_send_long_get( dmpi_send_handle, mpid_send_handle, 
				   actual_len );
#elif defined(MPID_USE_RNDV)
    rc = MPID_CH_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, 
				      actual_len );
#else
    rc = MPID_CH_post_send_long( dmpi_send_handle, mpid_send_handle, 
				   actual_len );
#endif
else
    rc = MPID_CH_post_send_short( dmpi_send_handle, mpid_send_handle, 
				    actual_len );

/* Poke the device in case there is data ... */
MPID_DRAIN_INCOMING;

return rc;
}

int MPID_CH_post_send_sync( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len, rc;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

if (actual_len > MPID_PKT_DATA_SIZE) 
#ifdef MPID_USE_RNDV
    rc = MPID_CH_post_send_long_rndv( dmpi_send_handle, mpid_send_handle, 
				      actual_len );
#elif defined(MPID_USE_GET)
    rc = MPID_CH_post_send_sync_long_get( dmpi_send_handle, mpid_send_handle, 
					  actual_len );
#else
    rc = MPID_CH_post_send_sync_long( dmpi_send_handle, mpid_send_handle, 
				      actual_len );
#endif
else
    rc = MPID_CH_post_send_sync_short( dmpi_send_handle, mpid_send_handle, 
					 actual_len );

/* Poke the device in case there is data ... */
MPID_DRAIN_INCOMING;

return rc;
}

int MPID_CH_Blocking_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_LIMITED_BUFFERS
/* Force the use of non-blocking operations so that head-to-head operations
   can complete when there is an IRECV posted */
dmpi_send_handle->dev_shandle.is_non_blocking = 1;
MPID_CH_post_send( dmpi_send_handle );
MPID_CH_complete_send( dmpi_send_handle );
dmpi_send_handle->dev_shandle.is_non_blocking = 0;
#else
MPID_CH_post_send( dmpi_send_handle );
MPID_CH_complete_send( dmpi_send_handle );
#endif
return MPI_SUCCESS;
}

/*
  Chameleon gets no asynchronous notice that the message has been complete,
  so there is no asynchronous ref to DMPI_mark_send_completed.
 */
int MPID_CH_isend_wait( dmpi_send_handle )
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
    while (!PInstatus(mpid_send_handle->sid))
	(void) MPID_CH_check_incoming( MPID_NOTBLOCKING );
#endif
    MPID_WSendChannel( (void *)0, mpid_send_handle->bytes_as_contig, -1,
		       mpid_send_handle->sid );
    mpid_send_handle->sid = 0;
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

  Deferred to a later implementation.
 */
int MPID_CH_complete_send( dmpi_send_handle ) 
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
	 MPID_CH_Cmpl_send_rndv( dmpi_send_handle );
         break;
#endif
#ifdef MPID_USE_GET
    case MPID_CMPL_SEND_GET:
	 MPID_CH_Cmpl_send_get( dmpi_send_handle );
         break;
#endif
#ifndef PI_NO_NSEND
    case MPID_CMPL_SEND_NB:
	 MPID_CH_Cmpl_send_nb( dmpi_send_handle );
         break;
#endif
    case MPID_CMPL_SEND_SYNC:
	 /* Also does non-blocking sync sends */
	 MPID_CH_Cmpl_send_sync( dmpi_send_handle );
	 break;
    default:
	 fprintf( stdout, "[%d]* Unexpected send completion mode %d\n", 
	          MPID_MyWorldRank, dmpi_send_handle->completer );
	 break;
    }
DEBUG_PRINT_MSG("S Exiting complete send")

return MPI_SUCCESS;
}

int MPID_Print_mode( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
char *modename=0;
int  sync_id=0;
switch (pkt->short_pkt.mode) {
    case MPID_PKT_SHORT:
    fputs( "short", fp );
    break;
    case MPID_PKT_LONG:
    fputs( "long", fp );
    break;
    case MPID_PKT_SHORT_SYNC:
    sync_id  = pkt->short_sync_pkt.sync_id;
    modename = "sync";
#ifndef MPID_USE_RNDV
    case MPID_PKT_LONG_SYNC:
    sync_id  = pkt->long_sync_pkt.sync_id;
    modename = "long sync";
    break;
#endif
    case MPID_PKT_SHORT_READY:
    fputs( "short ready", fp );
    break;
    case MPID_PKT_LONG_READY:
    fputs( "long ready", fp );
    break;
    case MPID_PKT_SYNC_ACK:
    modename = "syncack";
    sync_id = pkt->sync_ack_pkt.sync_id;
    case MPID_PKT_COMPLETE_SEND:
    fputs( "complete send", fp );
    break;
    case MPID_PKT_COMPLETE_RECV:
    fputs( "complete recv", fp );
    break;
    case MPID_PKT_REQUEST_SEND:
    fputs( "request send", fp );
    break;
    case MPID_PKT_OK_TO_SEND:
    fputs( "ok to send", fp );
    break;
    case MPID_PKT_READY_ERROR:
    fputs( "ready error", fp );
    break;
    case MPID_PKT_DO_GET:
    fputs( "do get", fp );
    break; 
    case MPID_PKT_DO_GET_SYNC:
    fputs( "do get sync", fp );
    break; 
    case MPID_PKT_DONE_GET:
    fputs( "done get", fp );
    break;
    case MPID_PKT_CONT_GET:
    fputs( "continue get", fp );
    break;
    default:
    fprintf( fp, "Mode %d is unknown!\n", pkt->short_pkt.mode );
    break;
    }
/* if (MPID_MODE_HAS_XDR(pkt)) fputs( "xdr", fp ); */

if (modename) {
    fputs( modename, fp );
    fprintf( fp, " - id = %d", sync_id );
    }
return MPI_SUCCESS;
}

/* 
   This routine tests for a send to be completed.  If non-blocking operations
   are used, it must check those operations...
 */
int MPID_CH_Test_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_USE_RNDV
if (!MPID_Test_handle(dmpi_send_handle) && dmpi_send_handle->dev_shandle.sid) {
    if (MPID_TestSendTransfer( dmpi_send_handle->dev_shandle.sid )) {
	/* If it is done, go ahead and mark the operation completed */
	MPID_EndSendTransfer( dmpi_send_handle->dev_shandle.start, 
			dmpi_send_handle->dev_shandle.bytes_as_contig, 
			     dmpi_send_handle->dest,
			     dmpi_send_handle->dev_shandle.recv_handle, 
	   dmpi_send_handle->dev_shandle.sid );
	dmpi_send_handle->dev_shandle.sid = 0;
	DMPI_mark_send_completed(dmpi_send_handle);
	}
    }
#endif
return MPID_Test_handle(dmpi_send_handle);
}

/* 
   This routine makes sure that we complete all pending requests

   Note: We should make it illegal here to receive anything put
   things like DONE_GET and COMPLETE_SEND.
 */
int MPID_CH_Complete_pending()
{
while (MPID_n_pending) {
    (void)MPID_CH_check_incoming( MPID_BLOCKING );
    }
return MPI_SUCCESS;
}

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
int MPID_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
fprintf( fp, "[%d] PKT =\n", MPID_MyWorldRank );
switch (pkt->head.mode) {
    case MPID_PKT_SYNC_ACK:
    fprintf( fp, "\
\tsync_id    = %d\n", pkt->sync_ack_pkt.sync_id );
    break; 
    case MPID_PKT_SHORT:
    case MPID_PKT_LONG:
    case MPID_PKT_SHORT_SYNC:
    case MPID_PKT_LONG_SYNC:
    case MPID_PKT_SHORT_READY:
    case MPID_PKT_LONG_READY:
fprintf( fp, "\
\tlen        = %d\n\
\ttag        = %d\n\
\tcontext_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank );
    break;
#ifdef MPID_USE_RNDV
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_REQUEST_SEND_READY:
    case MPID_PKT_OK_TO_SEND:
    MPID_CH_Rndv_print_pkt( fp, pkt );
    break;
#endif
#ifdef MPID_USE_GET
    case MPID_PKT_DO_GET:
    case MPID_PKT_DO_GET_SYNC:
    case MPID_PKT_DONE_GET:
    case MPID_PKT_CONT_GET:
    MPID_CH_Get_print_pkt( fp, pkt );
    break;
#endif
    default:
    fprintf( fp, "\n" );
    }
MPID_Print_mode( fp, pkt );
fputs( "\n", fp );
return MPI_SUCCESS;
}
#endif                  /* #DEBUG_END# */
