/*
 *  $Id: chrecv.c,v 1.33 1995/01/03 19:40:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: chrecv.c,v 1.33 1995/01/03 19:40:32 gropp Exp $";
#endif /* lint */

#include "mpid.h"

/* 
  This file contains the routines to handle receiving a message

  Because we don't know the length of messages (at least the long ones with
  tag MPID_PT2PT2_TAG(src) tags), we never post a receive.  Rather, we have
  a MPID_CMMD_check_incoming routine that looks for headers.  Note that
  messages sent from a source to destination with the MPID_PT2PT2_TAG(src)
  are ordered (we assume that the message-passing system preserves order).

  Possible Improvements:
  This current system does not "prepost" Irecv's, and so, on systems with
  aggressive delivery (like the Paragon), can suffer performance penalties.
  Obvious places for improvements are
     On blocking receives for long messages, post the irecv FIRST.
     If the message is actually short, cancel the receive.
         (potential problem - if the next message from the same source is
         long, it might match (incorrectly) with the posted irecv.  
         Possible fix: use sequence number for each processor pair, with
         the sequence numbers incremented for each message, short or long, 
         and place the sequence number into the tag field)

     For tags/sources/contexts in a specified range, post the irecv with
     at tag synthesized from all of the tag/source/context(/sequence number)
     May need to use cancel if the message is shorter than expected.
     This can be done with both blocking and non-blocking messages.

     Another approach is to generate "go-ahead" messages, to be handled in 
     chsend.c, perhaps by an interrupt-driven receive.  


     This file is changing to a system where each message kind is handled
     by its own routine (after the receive-queue matching).  This makes the
     number of lines of code larger, and generates some duplication of code,
     but also makes is significantly easier to tune each type (short, 
     synchronous, etc) to a particular protocol.
 */


/***************************************************************************/
/* These routines enable the debugging output                              */
/***************************************************************************/
static int DebugFlag = 0;

void MPID_SetDebugFlag( ctx, f )
void *ctx;
int f;
{
DebugFlag = f;
}
void MPID_SetRecvDebugFlag( ctx, f )
void *ctx;
int f;
{
DebugFlag = f;
MPID_SetSyncDebugFlag( ctx, f );
}
/***************************************************************************/

/***************************************************************************/
/* These are used to keep track of the number and kinds of messages that   */
/* are received                                                            */
/***************************************************************************/
#ifndef MPID_STAT_NONE
static int n_short       = 0,         /* short messages */
           n_long        = 0,         /* long messages */
           n_unexpected  = 0,         /* unexpected messages */
           n_syncack     = 0;         /* Syncronization acknowledgments */
#endif
static int DebugMsgFlag = 0;
void MPID_SetMsgDebugFlag( f )
int f;
{
DebugMsgFlag = f;
}
int MPID_GetMsgDebugFlag()
{
return DebugMsgFlag;
}
void MPID_PrintMsgDebug()
{
#ifndef MPID_STAT_NONE
fprintf( stdout, "[%d] short = %d, long = %d, unexpected = %d, ack = %d\n",
	 MPID_MyWorldRank, n_short, n_long, n_unexpected, n_syncack );
#endif
}
/***************************************************************************/

/***************************************************************************/
/* This is used to provide for a globally allocated message pkt in case
   we wish to preallocate or double buffer.  For example, the p4 device
   could use this to preallocate a message buffer; the Paragon could use
   this to use irecv's instead of recvs. 
 */
/***************************************************************************/
MPID_PKT_GALLOC

/* This routine is called by the initialization code to preform any 
   receiver initializations, such as preallocating or pre-posting a 
   control-message buffer
 */
void MPID_CMMD_Init_recv_code()
{
MPID_PKT_INIT();
}

/***************************************************************************/
/* These routines copy data from an incoming message into the provided     */
/* buffer.                                                                 */
/***************************************************************************/
int MPID_CMMD_Copy_body_short( dmpi_recv_handle, pkt, pktbuf )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
void         *pktbuf;
{
int          msglen;
int          err = MPI_SUCCESS;

MPID_KEEP_STAT(n_short++;)

msglen = pkt->head.len;
/* Check for truncation */
if (dmpi_recv_handle->dev_rhandle.bytes_as_contig < msglen) {
    err = MPI_ERR_TRUNCATE;
    (*MPID_ErrorHandler)( 1, "Truncated message"  );
    msglen = dmpi_recv_handle->dev_rhandle.bytes_as_contig;
    }
dmpi_recv_handle->totallen = msglen;
if (msglen > 0) 
    MEMCPY( dmpi_recv_handle->dev_rhandle.start, pktbuf, msglen ); 
DMPI_mark_recv_completed(dmpi_recv_handle);

return err;
}

int MPID_CMMD_Copy_body_sync_short( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
int err;

err = MPID_CMMD_Copy_body_short( dmpi_recv_handle, pkt, 
			       pkt->short_sync_pkt.buffer );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	    "[%d]SYNC Returning sync to %d with mode ", MPID_MyWorldRank,
	   from );
    MPID_Print_mode( MPID_DEBUG_FILE, pkt );
    fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
MPID_KEEP_STAT(n_syncack++;)
MPID_SyncReturnAck( pkt->short_sync_pkt.sync_id, from );

return err;
}

/* Now the long messages.  Only if not using the Rendevous protocol */
#ifndef MPID_USE_RNDV
/* 
    In the Rendevous version of this, it sends a request back to the
    sender for the data...
 */
int MPID_CMMD_Copy_body_long( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
MPID_RHANDLE *mpid_recv_handle;
int          msglen, err = MPI_SUCCESS;

mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
msglen           = pkt->head.len;

/* Check for truncation */
if (dmpi_recv_handle->dev_rhandle.bytes_as_contig < msglen) {
    err = MPI_ERR_TRUNCATE;
    (*MPID_ErrorHandler)( 1, "Truncated message"  );
    msglen = dmpi_recv_handle->dev_rhandle.bytes_as_contig;
    }
dmpi_recv_handle->totallen = msglen;
MPID_KEEP_STAT(n_long++;)
MPID_RecvFromChannel( mpid_recv_handle->start, msglen, from );
DMPI_mark_recv_completed(dmpi_recv_handle);

return err;
}

int MPID_CMMD_Copy_body_sync_long( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
int err;

err = MPID_CMMD_Copy_body_long( dmpi_recv_handle, pkt, from );

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	   "[%d]SYNC Returning sync to %d with mode ", MPID_MyWorldRank,
	   from );
    MPID_Print_mode( MPID_DEBUG_FILE, pkt );
    fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
MPID_KEEP_STAT(n_syncack++;)
MPID_SyncReturnAck( pkt->long_sync_pkt.sync_id, from );

return err;
}
#endif

/*
   This code is called when a receive finds that the message has already 
   arrived and has been placed in the unexpected queue.  This code
   stores the information about the message (source, tag, length),
   copies the message into the receiver's buffer, and generates a
   acknowledgement if the message has mode SYNC.

   dmpi_recv_handle is the API's receive handle that is to receive the
   data.

   dmpi_unexpected is the handle of the data found in the unexpected queue.

   In the case that the rendevous protocol is being used for long messages,
   we must begin the process of transfering the message.  Note that
   in this case, the message may not be completely transfered until
   we wait on the completion of the message.

   Note that in the Rendevous case, this routine may not set the
   completed field, since it the data may still be on its way.
   Because the Rendevous code is a rather different way of handling the
   processing of unexpected messages, there are two versions of this routine,
   one for MPID_USE_RNDV, and one without rendevous.  Make sure that you
   change the correct one (and both if there is a common problem!).
 */
#ifdef MPID_USE_RNDV
int MPID_CMMD_Process_unexpected( dmpi_recv_handle, dmpi_unexpected )
MPIR_RHANDLE *dmpi_recv_handle, *dmpi_unexpected;
{
MPID_RHANDLE *mpid_recv_handle;
MPID_RHANDLE *mpid_recv_handle_unex;
int err = MPI_SUCCESS;

MPID_KEEP_STAT(n_unexpected++;)

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	     "[%d]R Found message in unexpected queue (%s:%d)\n", 
	     MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* Copy relevant data to recv_handle */
mpid_recv_handle	   = &dmpi_recv_handle->dev_rhandle;
mpid_recv_handle_unex	   = &dmpi_unexpected->dev_rhandle;
dmpi_recv_handle->source   = dmpi_unexpected->source;
dmpi_recv_handle->tag	   = dmpi_unexpected->tag;
dmpi_recv_handle->totallen = mpid_recv_handle_unex->bytes_as_contig;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	    "[%d]R Found message in temp area of %d bytes (%s:%d)...\n", 
	    MPID_MyWorldRank, mpid_recv_handle_unex->bytes_as_contig,
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* Error test on length of message */
if (mpid_recv_handle->bytes_as_contig < dmpi_recv_handle->totallen) {
    mpid_recv_handle_unex->bytes_as_contig = mpid_recv_handle->bytes_as_contig;
    dmpi_recv_handle->totallen = mpid_recv_handle->bytes_as_contig;
    err = MPI_ERR_TRUNCATE;
    (*MPID_ErrorHandler)( 1, "Truncated message"  );
    }

    /* We need to see if the message has already been delivered or not.
       If it was short, it should already be here; otherwise, we need to 
       send a request for it.  Note that we give mpid_recv_handle, not
       mpid_recv_handle_unex here, since we will be testing mpid_recv_handle,
       not mpid_recv_handle_unex for completion.
     */
if (mpid_recv_handle_unex->send_id) {
    MPID_CMMD_Ack_Request( mpid_recv_handle, mpid_recv_handle_unex->from,
			 mpid_recv_handle_unex->send_id );
    }
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
  "[%d]R Copied message out of temp area; send mode is %x (%s:%d)..\n", 
	    MPID_MyWorldRank, mpid_recv_handle_unex->mode, 
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

if (mpid_recv_handle_unex->temp) {
    FREE( mpid_recv_handle_unex->temp );
    mpid_recv_handle_unex->temp = 0;      /* In case of a cancel */
    }

/* Return the synchronization message */
if (MPIR_MODE_IS_SYNC(mpid_recv_handle_unex)) {
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	fprintf( MPID_DEBUG_FILE,
       "[%d]SYNC Returning sync for %x to %d for rcv of unxpcted (%s:%d)\n", 
	       MPID_MyWorldRank,
	        mpid_recv_handle_unex->mode, mpid_recv_handle_unex->from,
	        __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
    MPID_KEEP_STAT(n_syncack++;)
    MPID_SyncReturnAck( mpid_recv_handle_unex->send_id, 
		        mpid_recv_handle_unex->from );
    }

if (!mpid_recv_handle_unex->send_id)
    DMPI_mark_recv_completed(dmpi_recv_handle);

/* Recover dmpi_unexpected.  This is ok even for the rendevous protocol 
   since all of the information needed has been transfered into 
   dmpi_recv_handle. 
 */
DMPI_free_unexpected( dmpi_unexpected );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE, 
	    "[%d]R Leaving 'process unexpected' (%s:%d)...\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
return err;
}

#else
int MPID_CMMD_Process_unexpected( dmpi_recv_handle, dmpi_unexpected )
MPIR_RHANDLE *dmpi_recv_handle, *dmpi_unexpected;
{
MPID_RHANDLE *mpid_recv_handle;
MPID_RHANDLE *mpid_recv_handle_unex;
int err = MPI_SUCCESS;

MPID_KEEP_STAT(n_unexpected++;)

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	     "[%d]R Found message in unexpected queue (%s:%d)\n", 
	     MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* Copy relevant data to recv_handle */
mpid_recv_handle	   = &dmpi_recv_handle->dev_rhandle;
mpid_recv_handle_unex	   = &dmpi_unexpected->dev_rhandle;
dmpi_recv_handle->source   = dmpi_unexpected->source;
dmpi_recv_handle->tag	   = dmpi_unexpected->tag;
dmpi_recv_handle->totallen = mpid_recv_handle_unex->bytes_as_contig;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	    "[%d]R Found message in temp area of %d bytes (%s:%d)...\n", 
	    MPID_MyWorldRank, mpid_recv_handle_unex->bytes_as_contig,
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* Error test on length of message */
if (mpid_recv_handle->bytes_as_contig < dmpi_recv_handle->totallen) {
    mpid_recv_handle_unex->bytes_as_contig = mpid_recv_handle->bytes_as_contig;
    dmpi_recv_handle->totallen = mpid_recv_handle->bytes_as_contig;
    err = MPI_ERR_TRUNCATE;
    (*MPID_ErrorHandler)( 1, "Truncated message"  );
    }

if (mpid_recv_handle_unex->bytes_as_contig > 0) {
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	fprintf( MPID_DEBUG_FILE, 
		"[%d]R About to copy to %x from %x (%s:%d)...\n", 
	       MPID_MyWorldRank,
	       mpid_recv_handle_unex->start, mpid_recv_handle_unex->temp,
	       __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
    MEMCPY( mpid_recv_handle->start, mpid_recv_handle_unex->temp,
	   mpid_recv_handle_unex->bytes_as_contig );
    }
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
  "[%d]R Copied message out of temp area; send mode is %x (%s:%d)..\n", 
	    MPID_MyWorldRank, mpid_recv_handle_unex->mode, 
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

if (mpid_recv_handle_unex->temp) {
    FREE( mpid_recv_handle_unex->temp );
    mpid_recv_handle_unex->temp = 0;      /* In case of a cancel */
    }

/* Return the synchronization message */
if (MPIR_MODE_IS_SYNC(mpid_recv_handle_unex)) {
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	fprintf( MPID_DEBUG_FILE,
       "[%d]SYNC Returning sync for %x to %d for rcv of unxpcted (%s:%d)\n", 
	       MPID_MyWorldRank,
	        mpid_recv_handle_unex->mode, mpid_recv_handle_unex->from,
	        __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
    MPID_KEEP_STAT(n_syncack++;)
    MPID_SyncReturnAck( mpid_recv_handle_unex->send_id, 
		        mpid_recv_handle_unex->from );
    }

DMPI_mark_recv_completed(dmpi_recv_handle);

/* Recover dmpi_unexpected.  This is ok even for the rendevous protocol 
   since all of the information needed has been transfered into 
   dmpi_recv_handle. 
 */
DMPI_free_unexpected( dmpi_unexpected );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE, 
	    "[%d]R Leaving 'process unexpected' (%s:%d)...\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
return err;
}
#endif

/*
   Post a receive.

   Since the Chameleon implementation lets the underlying message transport
   layer handle this, there isn't much to do.  Note that this is for 
   NONBLOCKING receives; there is a separate call for blocking receives.

   Otherwise, we simply try to handle any receives that are ready for
   processing.

   is_available is set if the message is already available (arrived before
   the message was posted).
 */
int MPID_CMMD_post_recv( dmpi_recv_handle, is_available ) 
MPIR_RHANDLE *dmpi_recv_handle;
int          *is_available;
{
MPIR_RHANDLE *dmpi_unexpected;
int          found, err;

/* If this is really a blocking receive, make the blocking receive code 
   do it... */
if (!dmpi_recv_handle->dev_rhandle.is_non_blocking) {
    *is_available = 1;
    return MPID_CMMD_blocking_recv( dmpi_recv_handle );
    }

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d]R starting recv for tag = %d, source = %d, ctx = %d, (%s:%d)\n", 
	    MPID_MyWorldRank, dmpi_recv_handle->tag, dmpi_recv_handle->source,
	    dmpi_recv_handle->contextid, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* At this time, we check to see if the message has already been received.
   (this is a macro that checks first to see if the queue is empty) */
DMPI_search_unexpected_queue( dmpi_recv_handle->source, 
		   dmpi_recv_handle->tag, dmpi_recv_handle->contextid, 
		   &found, 1, &dmpi_unexpected );
if (found) {
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	fprintf( MPID_DEBUG_FILE, "[%d]R found in unexpected queue (%s:%d)\n", 
	        MPID_MyWorldRank, __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
    *is_available = 1;
    return MPID_CMMD_Process_unexpected( dmpi_recv_handle, dmpi_unexpected );
    }
*is_available    = 0;

/* Add to the posted receive queue */
MPIR_enqueue( &MPIR_posted_recvs, dmpi_recv_handle, MPIR_QRHANDLE );

/* If we got here, the message is not yet available */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
      "[%d]R About to do a non-blocking check of incoming messages (%s:%d)\n",
	   MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

/* Process all pending messages until there are none left */
while (MPID_CMMD_check_incoming( MPID_NOTBLOCKING ) != -1) ;

/* Note that at this point, the message MAY be here by is_available is still 
   zero.  This is ok, since is_available is intended as an optimization */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE, "[%d]R Exiting post receive (%s:%d)\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

/* Return is_available instead??? */
return MPI_SUCCESS;
}

/*
   Copy the body of a message into the destination buffer for a posted
   receive.  This is used only when the matching receive exists and
   is described by dmpi_recv_handle.
 */
int MPID_CMMD_Copy_body( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
int err = MPI_SUCCESS;
switch (pkt->head.mode) {
    case MPID_PKT_SHORT_READY:
    case MPID_PKT_SHORT:
    err = MPID_CMMD_Copy_body_short( dmpi_recv_handle, pkt, 
				   pkt->short_pkt.buffer );
    DMPI_mark_recv_completed(dmpi_recv_handle);
    break;

    case MPID_PKT_SHORT_SYNC:
    /* sync_id  = pkt.short_sync_pkt.sync_id; */
    err = MPID_CMMD_Copy_body_sync_short( dmpi_recv_handle, pkt, from );
    DMPI_mark_recv_completed(dmpi_recv_handle);
    break;

#ifdef MPID_USE_RNDV
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_REQUEST_SEND_READY:
    /* Send back an OK to send */
    MPID_CMMD_Ack_Request( &dmpi_recv_handle->dev_rhandle, from, 
			 pkt->request_pkt.send_id );
    /* Note that in this case we do not mark the transfer as completed */
    break;
#else
    case MPID_PKT_LONG_READY:
    case MPID_PKT_LONG:
    err = MPID_CMMD_Copy_body_long( dmpi_recv_handle, pkt, from );
    DMPI_mark_recv_completed(dmpi_recv_handle);
    break;

    case MPID_PKT_LONG_SYNC:
    /* sync_id  = pkt.long_sync_pkt.sync_id; */
    err = MPID_CMMD_Copy_body_sync_long( dmpi_recv_handle, pkt, from );
    DMPI_mark_recv_completed(dmpi_recv_handle);
    break;
#endif
    }


return err;
}

/*
   Copy the body of a message into the destination buffer for an
   unexpected message.  The information on the message is stored in the
   dmpi_recv_handle, which has allocated by the DMPI_msg_arrived routine.

   Again, just as for Copy_body, in the rendevous case, this may not 
   complete the transfer, just begin it.
 */
#define MPIDGETMEM(len) \
            address = (char *)MALLOC(len);if(!address){\
	    (*MPID_ErrorHandler)( 1, \
			 "No more memory for storing unexpected messages"  );\
	    return MPI_ERR_EXHAUSTED; }

int MPID_CMMD_Copy_body_unex( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
MPID_RHANDLE *mpid_recv_handle;
char *address;
int  msglen;

mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
msglen           = pkt->head.len;

mpid_recv_handle->bytes_as_contig = msglen;
mpid_recv_handle->mode		  = 0;   
mpid_recv_handle->from		  = from;
mpid_recv_handle->send_id         = 0;
address				  = 0;
switch (pkt->head.mode) {
    case MPID_PKT_SHORT_READY:
    case MPID_PKT_SHORT:
	MPID_KEEP_STAT(n_short++;)
	if (msglen > 0) {
	    MPIDGETMEM(msglen);
	    MEMCPY( address, pkt->short_pkt.buffer, msglen );
	    }
	break;

    case MPID_PKT_SHORT_SYNC:
	/* Note that the sync_id may be a full address */
	mpid_recv_handle->mode	  = (int)MPIR_MODE_SYNCHRONOUS;
	mpid_recv_handle->send_id = pkt->short_sync_pkt.sync_id;
        MPID_KEEP_STAT(n_short++;)
	if (msglen > 0) {
	    MPIDGETMEM(msglen);
	    MEMCPY( address, pkt->short_sync_pkt.buffer, msglen );
	    }
	break;

#ifdef MPID_USE_RNDV
    case MPID_PKT_REQUEST_SEND:
    case MPID_PKT_REQUEST_SEND_READY:
	/* Save the send id.  In this case, there is no data. */
	dmpi_recv_handle->dev_rhandle.send_id = pkt->request_pkt.send_id;
	break;
#else
    case MPID_PKT_LONG_SYNC:
	/* Note that the sync_id may be a full address */
	mpid_recv_handle->mode	  = (int)MPIR_MODE_SYNCHRONOUS;
	mpid_recv_handle->send_id = pkt->long_sync_pkt.sync_id;
    /* Fall through to get data ... */
    case MPID_PKT_LONG_READY:
    case MPID_PKT_LONG:
	MPIDGETMEM(msglen);
	MPID_KEEP_STAT(n_long++;)
	MPID_RecvFromChannel( address, msglen, from );
	break;
#endif
    }
mpid_recv_handle->temp            = address;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag && (pkt->head.mode == MPID_PKT_SHORT_SYNC ||
    pkt->head.mode == MPID_PKT_LONG_SYNC)) {
    fprintf( MPID_DEBUG_FILE,
   "[%d]R setting mode of unexpected message to sync (%s:%d)\n", 
	   MPID_MyWorldRank, __FILE__, __LINE__ );
    }
#endif                  /* #DEBUG_END# */

#ifdef MPID_HAS_HETERO        /* #HETERO_START# */
if (MPID_PKT_HAS_XDR(pkt))
    dmpi_recv_handle->msgrep = MPIR_MSGREP_XDR;
else 
    dmpi_recv_handle->msgrep = MPIR_MSGREP_RECEIVER;
#endif                        /* #HETERO_END# */

#ifdef DEBUG_READY
if (MPID_MODE_IS_READY(pkt)) {
    (*MPID_ErrorHandler)( 1, 
			 "Received ready message without matching receive"  );
    return MPI_ERR_NOMATCH;
    }
#endif
return MPI_SUCCESS;
}

/***************************************************************************/
/* This is one of the main routines.  It checks for incoming messages and  */
/* dispatches them.  There is another such look in MPID_CMMD_blocking_recv   */
/* which is optimized for the important case of blocking receives for a    */
/* particular message.                                                     */
/***************************************************************************/

/* Check for incoming messages.
    Input Parameter:
.   is_blocking - true if this routine should block until a message is
    available

    Returns -1 if nonblocking and no messages pending

    This routine makes use of a single dispatch routine to handle all
    incoming messages.  This makes the code a little lengthy, but each
    piece is relatively simple.
 */    
int MPID_CMMD_check_incoming( is_blocking )
MPID_BLOCKING_TYPE is_blocking;
{
MPID_PKT_LALLOC	
int          from;
MPIR_RHANDLE *dmpi_recv_handle;
int          is_posted;
int          err = MPI_SUCCESS;

/* If nonblocking and no headers available, exit */
#ifndef pvm3
if (is_blocking == MPID_NOTBLOCKING) {
    if (!MPID_PKT_CHECK()) return -1;
    }
MPID_PKT_WAIT();
#else   /* #PVM3_START# */
/* pvm3.0 doesn't have a real probe, but what they do have meets the 
   semantics that we need here, though it is somewhat painful... 
   All this to save  the user a single routine call in the case where
   a probe is immediately followed by a recv.  Heaven help you if you
   use the probe to decide to call some other code to process the 
   message... 

   Later versions of PVM 3 may have a proper probe; if someone needs it,
   please send mail to mpi-bugs@mcs.anl.gov
*/
{
int bufid, bytes, msgtype; 
if (is_blocking == MPID_NOTBLOCKING) {
    if ((bufid = pvm_nrecv( -1, MPID_PT2PT_TAG )) <= 0) return -1;
    /* If we found a message, we now have to receive it */
    pvm_bufinfo( bufid, &bytes, &msgtype, &__PVMFROMTID );
    pvm_upkint( (int *)&pkt, bytes / sizeof(int), 1 );
    __PVMFROM = -1;
    }
else {
    /* For the blocking case, we can use the existing code ... */
    MPID_PKT_WAIT();
    }
}       /* #PVM3_END# */
#endif

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d]R received message (%s:%d)\n", MPID_MyWorldRank, __FILE__, __LINE__ );
    MPID_Print_packet( MPID_DEBUG_FILE, &pkt );
    }
#endif                  /* #DEBUG_END# */

/* Separate the incoming messages from control messages */
if (MPID_PKT_IS_MSG(MPID_PKT.head.mode)) {
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (DebugFlag) {
	fprintf( MPID_DEBUG_FILE,
"[%d]R rcvd msg for tag = %d, source = %d, ctx = %d, len = %d, mode = ", 
	       MPID_MyWorldRank, MPID_PKT.head.tag, from, 
	       MPID_PKT.head.context_id, MPID_PKT.head.len );
	MPID_Print_mode( MPID_DEBUG_FILE, &MPID_PKT );
	fprintf( MPID_DEBUG_FILE, "(%s:%d)\n", __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
/* Is the message expected or not? 
   This routine RETURNS a dmpi_recv_handle, creating one if the message 
   is unexpected (is_posted == 0) */
DMPI_msg_arrived( MPID_PKT.head.lrank, MPID_PKT.head.tag, 
		  MPID_PKT.head.context_id, 
                  &dmpi_recv_handle, &is_posted );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n", MPID_MyWorldRank, 
	    is_posted ? "posted" : "unexpected", __FILE__, __LINE__ );
    }
#endif                  /* #DEBUG_END# */
    if (is_posted) {
	/* We should check the size here for internal errors .... */
	switch (MPID_PKT.head.mode) {
	    case MPID_PKT_SHORT_READY:
	    case MPID_PKT_SHORT:
	    err = MPID_CMMD_Copy_body_short( dmpi_recv_handle, &MPID_PKT, 
				     (&MPID_PKT)->short_pkt.buffer );
	    break;
#ifdef MPID_USE_RNDV
	case MPID_PKT_REQUEST_SEND:
	case MPID_PKT_REQUEST_SEND_READY:
	    /* Send back an OK to send, with a tag value and 
	       a posted recv */
	    MPID_CMMD_Ack_Request( &dmpi_recv_handle->dev_rhandle, from, 
				 MPID_PKT.request_pkt.send_id );
	break;
#else
	case MPID_PKT_LONG_READY:
	case MPID_PKT_LONG:
	    err = MPID_CMMD_Copy_body_long( dmpi_recv_handle, &MPID_PKT, from );
	    break;
	case MPID_PKT_LONG_SYNC:
	    err = MPID_CMMD_Copy_body_sync_long( dmpi_recv_handle, &MPID_PKT, 
					       from );
	    break;
#endif
	case MPID_PKT_SHORT_SYNC:
	    err = MPID_CMMD_Copy_body_sync_short( dmpi_recv_handle, &MPID_PKT, 
					        from );
	    break;
	    }
	}
    else {
	MPID_CMMD_Copy_body_unex( dmpi_recv_handle, &MPID_PKT, from );
	}
    }
else {
    switch (MPID_PKT.head.mode) {
	case MPID_PKT_SYNC_ACK:
        MPID_SyncAck( MPID_PKT.sync_ack_pkt.sync_id, from );
	break;
	case MPID_PKT_COMPLETE_SEND:
	break;
	case MPID_PKT_COMPLETE_RECV:
	break;
#ifdef MPID_USE_RNDV
	case MPID_PKT_OK_TO_SEND:
	MPID_CMMD_Do_Request( MPID_PKT.sendok_pkt.use_tag, from, 
			    MPID_PKT.sendok_pkt.send_id );
	break;
#endif
	case MPID_PKT_READY_ERROR:
	break;
	default:
	fprintf( stdout, "Mode %d is unknown!\n", MPID_PKT.head.mode );
	}
    /* Really should remember error incase subsequent events are successful */
    }
return err;
}


/*
    This routine completes a particular receive.  It does this by processing
    incoming messages until the indicated message is received.

    For fairness, we may want a version with an array of handles.

    In the case of a rendevous send, it may need to wait on a nonblocking
    receive.

    NOTE: MANY MPI_TESTxxx routines are calling this when they
    should be calling MPID_Test_recv instead.  NEED TO FIX....
 */
int MPID_CMMD_complete_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
#if defined(MPID_USE_RNDV) && !defined(PI_NO_NRECV)
/* This will not work on stream devices unless we can guarentee that this
   message is the next one in the pipe.  Otherwise, we need a loop that
   does a check_incoming, interleaved with status checks of this
   message */
if (!dmpi_recv_handle->completed && dmpi_recv_handle->dev_rhandle.rid) {
    MPID_CMMD_Complete_Rndv( &dmpi_recv_handle->dev_rhandle );
    dmpi_recv_handle->completed = 1;
    return MPI_SUCCESS;
    }
#endif
while (!dmpi_recv_handle->completed) {
    (void)MPID_CMMD_check_incoming( MPID_BLOCKING );
    }
return MPI_SUCCESS;
}


/*
   Special case code for blocking receive.  The "common" case is handled with
   straight-through code; uncommon cases call routines.
   Note that this code never enqueues the request into the posted receive 
   queue.
 */
int MPID_CMMD_blocking_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
MPID_PKT_LALLOC
MPIR_RHANDLE *dmpi_unexpected, *dmpi_save_recv_handle;
int          found, from, is_posted, tag, source, context_id;
int          tagmask, srcmask;
int          ptag, pcid, plrk;   /* Values from packet */
int          err = MPI_SUCCESS;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
 "[%d]R starting blocking recv for tag = %d, source = %d, ctx = %d (%s:%d)\n", 
	    MPID_MyWorldRank, dmpi_recv_handle->tag, dmpi_recv_handle->source,
	    dmpi_recv_handle->contextid, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
/* At this time, we check to see if the message has already been received */
tag	   = dmpi_recv_handle->tag;
context_id = dmpi_recv_handle->contextid;
source	   = dmpi_recv_handle->source;

DMPI_search_unexpected_queue( source, tag, context_id, 
		   &found, 1, &dmpi_unexpected );
if (found) {
    return MPID_CMMD_Process_unexpected( dmpi_recv_handle, dmpi_unexpected );
    }

dmpi_save_recv_handle = dmpi_recv_handle;
/* If we got here, the message is not yet available */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    fprintf( MPID_DEBUG_FILE, 
	    "[%d]R Blocking recv; starting wait loop (%s:%d)\n", 
	    MPID_MyWorldRank, __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */
if (tag == MPI_ANY_TAG) {
    tagmask = 0;
    tag     = 0;
    }
else
    tagmask = ~0;
if (source == MPI_ANY_SOURCE) {
    srcmask = 0;
    source  = 0;
    }
else
    srcmask = ~0;
while (!dmpi_save_recv_handle->completed) {
    MPID_PKT_POST_AND_WAIT();
    if (MPID_PKT_IS_MSG(MPID_PKT.head.mode)) {
	ptag = MPID_PKT.head.tag;
	plrk = MPID_PKT.head.lrank;
	pcid = MPID_PKT.head.context_id;
	/* We should check the size here for internal errors .... */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	if (DebugFlag) {
	    fprintf( MPID_DEBUG_FILE,
      "[%d]R received message for tag = %d, source = %d, ctx = %d (%s:%d)\n", 
		   MPID_MyWorldRank, ptag, from, pcid, __FILE__, __LINE__ );
	    fflush( MPID_DEBUG_FILE );
	    }
#endif                  /* #DEBUG_END# */
	if (pcid == context_id        && 
	    (ptag & tagmask) == tag   &&
	    (plrk & srcmask) == source) {
	    /* Found the message that I'm waiting for (it was never queued) */
	    is_posted                = 1;
	    dmpi_recv_handle	     = dmpi_save_recv_handle;
	    dmpi_recv_handle->tag    = ptag;
	    dmpi_recv_handle->source = plrk;
	    }
	else {
	    /* Message other than the one we're waiting for... */
	    DMPI_msg_arrived( plrk, ptag, pcid, 
			     &dmpi_recv_handle, &is_posted );
	    }
#ifdef MPID_HAS_HETERO          /* #HETERO_START# */
	/* Look for XDR bit */
	if (MPID_PKT_HAS_XDR(&MPID_PKT)) 
	    dmpi_recv_handle->msgrep = MPIR_MSGREP_XDR;
	else
	    dmpi_recv_handle->msgrep = MPIR_MSGREP_RECEIVER;
#endif                          /* #HETERO_END# */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
	if (DebugFlag) {
	    fprintf( MPID_DEBUG_FILE,
		    "[%d]R msg was %s (%s:%d)\n", MPID_MyWorldRank, 
		   is_posted ? "posted" : "unexpected", __FILE__, __LINE__ );
	    }
#endif                  /* #DEBUG_END# */
	if (is_posted) {
	    err = MPID_CMMD_Copy_body( dmpi_recv_handle, &MPID_PKT, from );
#ifdef MPID_USE_RNDV
	    /* In the special case that we have received the message that
	       we are looking for, but it was sent with the Rendevous
	       send, we need to wait for the message to complete */
	    if (dmpi_recv_handle == dmpi_save_recv_handle &&
		!dmpi_recv_handle->completed && !dmpi_recv_handle->rid) {
		MPID_CMMD_Complete_recv( dmpi_recv_handle );
		return err;
		}
#endif
	    }
	else {
	    MPID_CMMD_Copy_body_unex( dmpi_recv_handle, &MPID_PKT, from );
	    }
	}
    else {
	switch (MPID_PKT.head.mode) {
	    case MPID_PKT_SYNC_ACK:
	    MPID_SyncAck( MPID_PKT.sync_ack_pkt.sync_id, from );
	    break;
	    case MPID_PKT_COMPLETE_SEND:
	    break;
	    case MPID_PKT_COMPLETE_RECV:
	    break;
#ifdef MPID_USE_RNDV
	    case MPID_PKT_OK_TO_SEND:
	    /* Lookup send handle, respond with data on given tag */
	    MPID_CMMD_Do_Request( MPID_PKT.sendok_pkt.use_tag, from, 
			        MPID_PKT.sendok_pkt.send_id );
	    break;
#endif
	    case MPID_PKT_READY_ERROR:
	    break;
	    default:
	    fprintf( stdout, "Mode %d is unknown!\n", MPID_PKT.head.mode );
	    }
	}
    }
return err;
}

#ifdef MPID_USE_RNDV
static int CurTag = 1;
static int TagsInUse = 0;

/* Respond to a request to send a message when the message is found to
   be posted */
int MPID_CMMD_Ack_Request( mpid_recv_handle, from, send_id )
MPID_RHANDLE *mpid_recv_handle;
int          from;
MPID_Aint    send_id;
{
int                   tag;
MPID_PKT_OK_TO_SEND_T pkt;

/* Generate a tag */
MPID_NewChannel( from, &tag );
TagsInUse++;
#ifndef PI_NO_NRECV
/* Post the non-blocking receive */
MPID_IRecvFromChannel( mpid_recv_handle->start, 
		       mpid_recv_handle->bytes_as_contig, tag, 
		       mpid_recv_handle->rid );
#endif

pkt.mode    = MPID_PKT_OK_TO_SEND;
pkt.send_id = send_id;
pkt.use_tag = tag;

/* Send a message back with the tag in it */
MPID_SendControl( &pkt, sizeof(MPID_PKT_OK_TO_SEND_T), from );
return MPI_SUCCESS;
}

MPID_CMMD_Complete_Rndv( mpid_recv_handle )
MPID_RHANDLE *mpid_recv_handle;
{
if (--TagsInUse == 0) CurTag = 1;
MPID_WRecvFromChannel( 0, 0, 0, mpid_recv_handle->rid );
mpid_recv_handle->rid = 0;
}

/* Fullfill a request for a message */
int MPID_CMMD_Do_Request( use_tag, from, send_id )
int          use_tag, from;
MPID_Aint    send_id;
{
MPID_SHANDLE *mpid_send_handle;
MPIR_SHANDLE *dmpi_send_handle;

/* Find the send operation (check that it hasn't been cancelled!) */
dmpi_send_handle = (MPIR_SHANDLE *)send_id;
mpid_send_handle = &dmpi_send_handle->dev_shandle;
MPID_IRRSendChannel( dmpi_send_handle->dev_shandle.start,
	   dmpi_send_handle->dev_shandle.bytes_as_contig, use_tag, from, 
	   mpid_send_handle->sid );
return MPI_SUCCESS;
}
#endif
