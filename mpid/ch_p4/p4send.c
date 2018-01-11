#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_HAS_HETERO


/*
 *  $Id: chsend.c,v 1.26 1994/11/08 16:00:35 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char SCCSid[] = "%W% %G%";
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

/***************************************************************************/
/* These routines enable the debugging output                              */
/***************************************************************************/
static int DebugFlag = 0;   /* Set to 0 for no debug output */

void MPID_SetSendDebugFlag( ctx, f )
void *ctx;
int f;
{
DebugFlag = f;
}
/***************************************************************************/

/* This routine is a hook for eventually allowing pre-initialized packets */
void MPID_P4_Init_send_code()
{
}

/* 
   This file includes the routines to handle the device part of a send
   for Chameleon

   As a reminder, the first element is the device handle, the second is
   the (basically opaque) mpi handle
 */

/* Send a short (single packet message) */
int MPID_P4_post_send_short( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SHORT_T pkt;
int              dest;

/* These references are ordered to match the order they appear in the 
   structure */
dest             = dmpi_send_handle->dest;
pkt.mode	 = MPID_PKT_SHORT; 
pkt.context_id	 = dmpi_send_handle->contextid;
pkt.lrank	 = dmpi_send_handle->lrank;
pkt.tag		 = dmpi_send_handle->tag;
pkt.len          = len;
#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    pkt.has_xdr = MPID_MODE_XDR;
#endif                         /* #HETERO_END# */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
 "[%d]S Starting a send of tag = %d, len = %d, ctx = %d, dest = %d, mode=",
	    __MYPROCID, pkt.tag, pkt.len, pkt.context_id, dest );
    MPID_Print_mode( stdout, (MPID_PKT_T*)&pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */

#ifdef MPID_HAS_HETERO       /* #HETERO_START# */
/* Convert the header into canonical integer order */
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    /* Need to swap to receiver's order.  We ALWAYS reorder at the
       sender's end (this is because a message can be received with MPI_Recv
       instead of MPI_Recv/MPI_Unpack, and hence requires us to use a format
       that matches the receiver's ordering without requiring a user-unpack.
       We may need to generalize this for XDR systems... */
    SY_ByteSwapInt((int*)&pkt,sizeof(MPID_PKT_HEAD_T) / sizeof(int) );
    }
#endif                       /* #HETERO_END# */

if (len > 0) {
    MEMCPY( pkt.buffer, mpid_send_handle->start, len );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	MPID_P4_Print_pkt_data( "S Getting data from mpid->start", 
			        pkt.buffer, len );
	}
#endif                  /* #DEBUG_END# */
    }
/* Always use a blocking send for short messages.
   (May fail with systems that do not provide adequate
   buffering.  These systems should switch to non-blocking sends)
 */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( "[%d]S Sending message in a single packet (%s:%d)...\n", 
	   __MYPROCID, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)&pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
/* In case the message is marked as non-blocking, indicate that we don't
   need to wait on it.  We may also want to use nonblocking operations
   to send the envelopes.... */
mpid_send_handle->sid = 0;
#ifdef MPID_TINY_BUFFERS
/* In the event that any blocking send might block until the message is
   received rather than until the message is buffered, check incoming 
   messages.  Do this only for non_blocking messages. */
if (mpid_send_handle->is_non_blocking) 
    while (MPID_P4_check_incoming( MPID_NOTBLOCKING ) != -1) ;
#endif
p4_sendx(MPID_PT2PT_TAG,dest,(char *)(&pkt),len + sizeof(MPID_PKT_HEAD_T),P4NOX );
DMPI_mark_send_completed( dmpi_send_handle );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( "[%d]S Sent message in a single packet (%s:%d)...\n", 
	    __MYPROCID, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
return MPI_SUCCESS;
}

/* Long message */
int MPID_P4_post_send_long( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
char             *address;
#ifdef MPID_USE_RNDV
MPID_PKT_REQUEST_SEND_T pkt;
#else
MPID_PKT_LONG_T  pkt;
#endif
int              dest;

#ifdef MPID_USE_RNDV
pkt.mode       = MPID_PKT_REQUEST_SEND;
pkt.send_id    = (MPID_Aint) dmpi_send_handle;
#else
pkt.mode       = MPID_PKT_LONG; 
#endif
pkt.context_id = dmpi_send_handle->contextid;
pkt.lrank      = dmpi_send_handle->lrank;
pkt.tag	       = dmpi_send_handle->tag;
pkt.len	       = len;
#ifdef MPID_HAS_HETERO      /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    pkt.has_xdr = MPID_MODE_XDR;
#endif                      /* #HETERO_END# */
dest           = dmpi_send_handle->dest;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
 "[%d]S Starting a send of tag = %d, len = %d, ctx = %d, dest = %d, mode=",
	    __MYPROCID, pkt.tag, pkt.len, pkt.context_id, dest );
    MPID_Print_mode( stdout, (MPID_PKT_T*)&pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */

#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
/* Convert the header into canonical integer order */
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    /* Need to swap to receivers order.  We ALWAYS reorder at the
       sender's end */
    SY_ByteSwapInt((int *)&pkt,sizeof(MPID_PKT_HEAD_T) / sizeof(int) );
    }
#endif                         /* #HETERO_END# */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",
	   __MYPROCID, *(int *)mpid_send_handle->start, __FILE__, __LINE__ );
    printf( "[%d]S Sending extra-long message (%s:%d)...\n", 
	    __MYPROCID, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)&pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
/* Send as packet only */
#ifdef MPID_TINY_BUFFERS
/* In the event that any blocking send might block until the message is
   received rather than until the message is buffered, check incoming 
   messages.  */
if (mpid_send_handle->is_non_blocking) 
    while (MPID_P4_check_incoming( MPID_NOTBLOCKING ) != -1) ;
#endif
p4_sendx(MPID_PT2PT_TAG,dest,(char *)(&pkt),sizeof(MPID_PKT_LONG_T),P4NOX );

#ifndef MPID_USE_RNDV
/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
#ifndef PI_NO_NSEND
if (mpid_send_handle->is_non_blocking) {
    {mpid_send_handle->sid =0;p4_sendx(MPID_PT2PT2_TAG(__MYPROCID),dest,(char *)(address),len,P4NOX);};
    }
else 
#endif
    {
    mpid_send_handle->sid = 0;
    p4_sendx(MPID_PT2PT2_TAG(__MYPROCID),dest,(char *)(address),len,P4NOX );
    DMPI_mark_send_completed( dmpi_send_handle );
    }
#endif
return MPI_SUCCESS;
}

int MPID_P4_post_send_sync_short( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SHORT_SYNC_T pkt;
int                   dest;

/* These references are ordered to match the order they appear in the 
   structure */
dest             = dmpi_send_handle->dest;
pkt.mode	 = MPID_PKT_SHORT_SYNC; 
pkt.context_id	 = dmpi_send_handle->contextid;
pkt.lrank	 = dmpi_send_handle->lrank;
pkt.tag		 = dmpi_send_handle->tag;
pkt.len          = len;
#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    pkt.has_xdr = MPID_MODE_XDR;
#endif                         /* #HETERO_END# */
pkt.sync_id      = MPID_P4_Get_Sync_Id( dmpi_send_handle, mpid_send_handle );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
 "[%d]S Starting a send of tag = %d, len = %d, ctx = %d, dest = %d, mode=",
	    __MYPROCID, pkt.tag, pkt.len, pkt.context_id, dest );
    MPID_Print_mode( stdout, (MPID_PKT_T*)&pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */

#ifdef MPID_HAS_HETERO       /* #HETERO_START# */
/* Convert the header into canonical integer order */
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    /* Need to swap to receiver's order.  We ALWAYS reorder at the
       sender's end (this is because a message can be received with MPI_Recv
       instead of MPI_Recv/MPI_Unpack, and hence requires us to use a format
       that matches the receiver's ordering without requiring a user-unpack.
       We may need to generalize this for XDR systems... */
    SY_ByteSwapInt((int *)&pkt,sizeof(MPID_PKT_HEAD_T) / sizeof(int) + 1 );
    }
#endif                       /* #HETERO_END# */

if (len > 0) {
    MEMCPY( pkt.buffer, mpid_send_handle->start, len );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	MPID_P4_Print_pkt_data( "S Getting data from mpid->start", 
			        pkt.buffer, len );
	}
#endif                  /* #DEBUG_END# */
    }
/* Always use a blocking send for short messages.
   (May fail with systems that do not provide adequate
   buffering.  These systems should switch to non-blocking sends)
 */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( "[%d]S Sending message in a single packet (%s:%d)...\n", 
	   __MYPROCID, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)&pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
/* In case the message is marked as non-blocking, indicate that we don't
   need to wait on it */
mpid_send_handle->sid = 0;
p4_sendx(MPID_PT2PT_TAG,dest,(char *)(&pkt),len + (sizeof(MPID_PKT_SHORT_SYNC_T)-MPID_PKT_MAX_DATA_SIZE),P4NOX );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( "[%d]S Sent message in a single packet (%s:%d)...\n", 
	    __MYPROCID, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
return MPI_SUCCESS;
}

/* Long message */
int MPID_P4_post_send_sync_long( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
char                 *address;
MPID_PKT_LONG_SYNC_T pkt;
int                  dest;

pkt.mode       = MPID_PKT_LONG_SYNC; 
pkt.context_id = dmpi_send_handle->contextid;
pkt.lrank      = dmpi_send_handle->lrank;
pkt.tag	       = dmpi_send_handle->tag;
pkt.len	       = len;
#ifdef MPID_HAS_HETERO      /* #HETERO_START# */
if (dmpi_send_handle->msgrep == MPIR_MSGREP_XDR) 
    pkt.has_xdr = MPID_MODE_XDR;
#endif                      /* #HETERO_END# */
dest           = dmpi_send_handle->dest;
pkt.sync_id    = MPID_P4_Get_Sync_Id( dmpi_send_handle, mpid_send_handle );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
 "[%d]S Starting a send of tag = %d, len = %d, ctx = %d, dest = %d, mode=",
	    __MYPROCID, pkt.tag, pkt.len, pkt.context_id, dest );
    MPID_Print_mode( stdout, (MPID_PKT_T*)&pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */

#ifdef MPID_HAS_HETERO         /* #HETERO_START# */
/* Convert the header into canonical integer order */
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    /* Need to swap to receivers order.  We ALWAYS reorder at the
       sender's end */
    SY_ByteSwapInt((int *)&pkt,sizeof(MPID_PKT_HEAD_T) / sizeof(int) + 1 );
    }
#endif                         /* #HETERO_END# */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
	   "[%d]S Getting data from mpid->start, first int is %d (%s:%d)\n",
	   __MYPROCID, *(int *)&mpid_send_handle->start, 
	   __FILE__, __LINE__ );
    printf( "[%d]S Sending extra-long message (%s:%d)...\n", 
	    __MYPROCID, __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T*)&pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
/* Send as packet only */
p4_sendx(MPID_PT2PT_TAG,dest,(char *)(&pkt),sizeof(MPID_PKT_LONG_SYNC_T),P4NOX );

/* Send the body of the message */
address    = ((char*)mpid_send_handle->start);
#ifndef PI_NO_NSEND
if (mpid_send_handle->is_non_blocking) {
    {mpid_send_handle->sid =0;p4_sendx(MPID_PT2PT2_TAG(__MYPROCID),dest,(char *)(address),len,P4NOX);};
    }
else 
#endif
    {
    mpid_send_handle->sid = 0;
    p4_sendx(MPID_PT2PT2_TAG(__MYPROCID),dest,(char *)(address),len,P4NOX );
    }
return MPI_SUCCESS;
}


/*
   This implementation immediately sends the data.

   It takes advantage of being provided with the address of the user-buffer
   in the contiguous case.
 */
int MPID_P4_post_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

if (actual_len > MPID_PKT_DATA_SIZE) 
    return MPID_P4_post_send_long( dmpi_send_handle, mpid_send_handle, 
				   actual_len );
else
    return MPID_P4_post_send_short( dmpi_send_handle, mpid_send_handle, 
				    actual_len );
}

int MPID_P4_post_send_sync( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle;
int         actual_len;

mpid_send_handle = &dmpi_send_handle->dev_shandle;
actual_len       = mpid_send_handle->bytes_as_contig;

if (actual_len > MPID_PKT_DATA_SIZE) 
#ifdef MPID_USE_RNDV
    return MPID_P4_post_send_long( dmpi_send_handle, mpid_send_handle, 
				   actual_len );
#else
    return MPID_P4_post_send_sync_long( dmpi_send_handle, mpid_send_handle, 
				        actual_len );
#endif
else
    return MPID_P4_post_send_sync_short( dmpi_send_handle, mpid_send_handle, 
					 actual_len );
}

int MPID_P4_Blocking_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_LIMITED_BUFFERS
/* Force the use of non-blocking operations so that head-to-head operations
   can complete when there is an IRECV posted */
dmpi_send_handle->dev_shandle.is_non_blocking = 1;
MPID_P4_post_send( dmpi_send_handle );
MPID_P4_complete_send( dmpi_send_handle );
dmpi_send_handle->dev_shandle.is_non_blocking = 0;
#else
MPID_P4_post_send( dmpi_send_handle );
MPID_P4_complete_send( dmpi_send_handle );
#endif
return MPI_SUCCESS;
}

/*
  Chameleon gets no asynchronous notice that the message has been complete,
  so there is no asynchronous ref to DMPI_mark_send_completed.
 */
int MPID_P4_isend_wait( dmpi_send_handle )
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
    while (!1)
	(void) MPID_P4_check_incoming( MPID_NOTBLOCKING );
#endif
    ;
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
int MPID_P4_complete_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;

/* Check to see if we need to complete the send. */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
	printf( "[%d]S Entering complete send (%s:%d)...\n", 
	        __MYPROCID, __FILE__, __LINE__ );
	fflush( stdout );
	}
#endif                  /* #DEBUG_END# */

/* If we have rendevous send, then we may need to first wait until the
   message has been requested; then wait on the send to complete... */
#ifdef MPID_USE_RNDV
while (!dmpi_send_handle->completed && mpid_send_handle->sid == 0)
    /* This can be a BLOCKING check because we must wait until
       an "ok to send" message arrives */
    (void) MPID_P4_check_incoming( MPID_BLOCKING );
#endif

if (mpid_send_handle->sid)  {
    /* Before we do the wait, try to clear all pending messages */
    (void)MPID_P4_check_incoming( MPID_NOTBLOCKING );
    MPID_P4_isend_wait( dmpi_send_handle );
    }
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
	printf( "[%d]S Entering complete send while loop (%s:%d)...\n", 
	        __MYPROCID, __FILE__, __LINE__ );
	fflush( stdout );
	}
#endif                  /* #DEBUG_END# */
while (!dmpi_send_handle->completed) {
    /* This waits for the completion of a synchronous send, since at
       this point, we've finished waiting for the {=0;p4_sendx(,,(char *)(),,);} to complete */
    (void)MPID_P4_check_incoming( MPID_BLOCKING );
    }
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( "[%d]S Exiting complete send (%s:%d)...\n", 
	   __MYPROCID, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
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
    case MPID_PKT_LONG:
    break;
    case MPID_PKT_SHORT_SYNC:
    sync_id  = pkt->short_sync_pkt.sync_id;
    modename = "sync";
#ifndef MPID_USE_RNDV
    case MPID_PKT_LONG_SYNC:
    sync_id  = pkt->long_sync_pkt.sync_id;
    modename = "sync";
    break;
#endif
    case MPID_PKT_SHORT_READY:
    case MPID_PKT_LONG_READY:
    fputs( "ready", fp );
    break;
    case MPID_PKT_SYNC_ACK:
    modename = "syncack";
    sync_id = pkt->sync_ack_pkt.sync_id;
    case MPID_PKT_COMPLETE_SEND:
    break;
    case MPID_PKT_COMPLETE_RECV:
    break;
    case MPID_PKT_REQUEST_SEND:
    break;
    case MPID_PKT_OK_TO_SEND:
    break;
    case MPID_PKT_READY_ERROR:
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
int MPID_P4_Test_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
#ifdef MPID_USE_RNDV
if (!dmpi_send_handle->completed && dmpi_send_handle->dev_shandle.sid) {
    if (PInstatis(  dmpi_send_handle->dev_shandle.sid )) {
	/* If it is done, go ahead and mark the operation completed */
	;
	dmpi_send_handle->dev_shandle.sid = 0;
	dmpi_send_handle->completed	  = 1;
	}
    }
#endif
return dmpi_send_handle->completed;
}

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
int MPID_Print_packet( fp, pkt )
FILE        *fp;
MPID_PKT_T  *pkt;
{
fprintf( fp, "[%d] PKT =\n", __MYPROCID );
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
\tcontent_id = %d\n\
\tlrank      = %d\n\
\tmode       = ", 
	pkt->head.len, pkt->head.tag, pkt->head.context_id, pkt->head.lrank );
    break;
    default:
    fprintf( fp, "\n" );
    }
MPID_Print_mode( fp, pkt );
fputs( "\n", fp );
return MPI_SUCCESS;
}
#endif                  /* #DEBUG_END# */
