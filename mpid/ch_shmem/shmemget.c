/*
 *  $Id: chget.c,v 1.2 1995/07/25 02:40:50 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: chget.c,v 1.2 1995/07/25 02:40:50 gropp Exp gropp $";
#endif /* lint */

#include "mpid.h"

/* 
  This file contains the routines to handle trnasfering messages with 
  a "get" or "put" protocol (only get enabled so far).

 */

/* Here are some definitions to simplify debugging */
#include "mpiddebug.h"
/***************************************************************************/

/***************************************************************************/
/* These are used to keep track of the number and kinds of messages that   */
/* are received                                                            */
/***************************************************************************/
#include "mpidstat.h"

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are outstanding                                      */
/***************************************************************************/
extern int MPID_n_pending;  /* Number of uncompleted split requests */

/***************************************************************************/
/* This is used to provide for a globally allocated message pkt in case
   we wish to preallocate or double buffer.  For example, the p4 device
   could use this to preallocate a message buffer; the Paragon could use
   this to use irecv's instead of recvs. 
 */
/***************************************************************************/
MPID_PKT_GALLOC


#ifdef MPID_USE_GET

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
int MPID_SHMEM_Process_unexpected_get( dmpi_recv_handle, dmpi_unexpected )
MPIR_RHANDLE *dmpi_recv_handle, *dmpi_unexpected;
{
MPID_RHANDLE *mpid_recv_handle;
MPID_RHANDLE *mpid_recv_handle_unex;
int err = MPI_SUCCESS;

MPID_KEEP_STAT(MPID_n_unexpected++;)

DEBUG_PRINT_MSG("R Found message in unexpected queue")

/* It is possible that the message has not yet arrived.  We may even want
   to go get it.  Test for that case */
MPID_SHMEM_complete_recv( dmpi_unexpected );

/* Copy relevant data to recv_handle */
mpid_recv_handle	   = &dmpi_recv_handle->dev_rhandle;
mpid_recv_handle_unex	   = &dmpi_unexpected->dev_rhandle;
dmpi_recv_handle->source   = dmpi_unexpected->source;
dmpi_recv_handle->tag	   = dmpi_unexpected->tag;
dmpi_recv_handle->totallen = mpid_recv_handle_unex->bytes_as_contig;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
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
    MEMCPY( mpid_recv_handle->start, mpid_recv_handle_unex->temp,
	   mpid_recv_handle_unex->bytes_as_contig );
    }
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
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
    if (MPID_DebugFlag) {
	fprintf( MPID_DEBUG_FILE,
       "[%d]SYNC Returning sync for %x to %d for rcv of unxpcted (%s:%d)\n", 
	       MPID_MyWorldRank,
	        mpid_recv_handle_unex->mode, mpid_recv_handle_unex->from,
	        __FILE__, __LINE__ );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */
    MPID_KEEP_STAT(MPID_n_syncack++;)
    MPID_SyncReturnAck( mpid_recv_handle_unex->send_id, 
		        mpid_recv_handle_unex->from );
    }

DMPI_mark_recv_completed(dmpi_recv_handle);

/* Recover dmpi_unexpected.  This is ok even for the rendevous protocol 
   since all of the information needed has been transfered into 
   dmpi_recv_handle. 
 */
DMPI_free_unexpected( dmpi_unexpected );

DEBUG_PRINT_MSG("R Leaving 'process unexpected'")

return err;
}

int MPID_SHMEM_Save_unex_get( dmpi_recv_handle, pkt, from )
MPIR_RHANDLE *dmpi_recv_handle;
MPID_PKT_T   *pkt;
int          from;
{
MPID_RHANDLE *mpid_recv_handle;

mpid_recv_handle		      = &dmpi_recv_handle->dev_rhandle;
dmpi_recv_handle->dev_rhandle.send_id = pkt->get_pkt.send_id;
dmpi_recv_handle->totallen	      = pkt->get_pkt.len;
mpid_recv_handle->mode		      = (int)MPIR_MODE_SYNCHRONOUS;
mpid_recv_handle->send_id	      = pkt->get_pkt.sync_id;
return MPI_SUCCESS;
}

/*
    This code provides routines for performing get-operation copies.
    This code allows partial data to be returned; the packet is returned
    to the sender when the transfer is completed (as a control packet).

    A receive get happens in two parts.
    If the recv_id field in the packet is null, then this is the first
    time we've seen this packet.  Otherwise, the recv_id gives us the
    address of the matching receive handle.  

    The address is the address to get from; len_avail is the amount to
    copy.  When the copy is complete, the mode is changed to MPID_PKT_DONE_GET
    and sent back the the sender.

    It turns out that to send the same packet back in the case where the 
    packets are dynamically allocated is a bit too tricky for the current
    implementation.  One possibility is an "in_use" bit; this requires that
    any packet that is returned eventually comes back.  Another is to keep
    track of whether the packet should be free; perhaps by inlining the 
    "DO_GET" code (then we can use the MPID_PKT_RECV_CLR(pkt) call).
    Left as an exercise for the reader.
 */
int MPID_SHMEM_Do_get( dmpi_recv_handle, from, pkt )
MPIR_RHANDLE   *dmpi_recv_handle;
MPID_PKT_GET_T *pkt;
int            from;
{
int msglen, err;

msglen = pkt->len;
MPID_SHMEMK_MSGLEN(dmpi_recv_handle,msglen,err)
dmpi_recv_handle->totallen = msglen;
pkt->recv_id = (MPID_Aint) dmpi_recv_handle;
err = MPID_SHMEM_Do_get_to_mem( dmpi_recv_handle->dev_rhandle.start, from, pkt );

if (pkt->cur_offset >= pkt->len) {
    DMPI_mark_recv_completed(dmpi_recv_handle);
#ifdef MPID_DEBUG_ALL /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE, 
	     "[%d] Do Get completed read of data (tag = %d, left = %d)\n", 
	     MPID_MyWorldRank, dmpi_recv_handle->tag,
	    pkt->len - pkt->cur_offset );
    fflush( MPID_DEBUG_FILE );
    }
#endif                /* #DEBUG_END# */
    }
else
    dmpi_recv_handle->completer = MPID_CMPL_RECV_GET;
return err;
}

int MPID_SHMEM_Do_get_to_mem( address, from, pkt )
void           *address;
MPID_PKT_GET_T *pkt;
int            from;
{
#if defined(MPID_PKT_DYNAMIC_RECV)
MPID_PKT_SEND_DECL(MPID_PKT_GET_T,tpkt);
#endif

MEMCPY( address, pkt->address, pkt->len_avail );

pkt->cur_offset += pkt->len_avail;

#if !defined(MPID_PKT_GET_NEEDS_ACK)
if (pkt->len - pkt->cur_offset > 0) {
#endif

#if defined(MPID_PKT_DYNAMIC_RECV)
MPID_PKT_SEND_ALLOC(MPID_PKT_GET_T,tpkt);
MEMCPY( MPID_PKT_SEND_ADDR(tpkt), pkt, sizeof(MPID_PKT_GET_T) );
MPID_PKT_SEND_SET(tpkt,mode,MPID_PKT_DONE_GET);
MPID_SendControl( MPID_PKT_SEND_ADDR(tpkt), sizeof(MPID_PKT_GET_T), from );
MPID_PKT_SEND_FREE(tpkt);
#else
pkt->mode = MPID_PKT_DONE_GET;
MPID_SendControl( pkt, sizeof(MPID_PKT_GET_T), from );
#endif

#ifdef MPID_DEBUG_ALL /* #DEBUG_START# */
if (MPID_DebugFlag) {
    MPIR_RHANDLE *dmpi_recv_handle = (MPIR_RHANDLE *)(pkt->recv_id);
    fprintf( MPID_DEBUG_FILE, 
	     "[%d] Do Get mem completed read of data (tag = %d, left=%d)\n", 
	     MPID_MyWorldRank, dmpi_recv_handle->tag, 
	    pkt->len - pkt->cur_offset );
    fflush( MPID_DEBUG_FILE );
    }
#endif                /* #DEBUG_END# */

#if !defined(MPID_PKT_GET_NEEDS_ACK)
}
else {
    MPID_FreeGetAddress( pkt->address );
    }
#endif

/* NOTE IF WE SEND THE PACKET BACK, WE MUST NOT FREE IT!!! IN THE RECV CODE */

return MPI_SUCCESS;
}

/* 
  Handle the continuation of a get (partial data transmission) 
 */
int MPID_SHMEM_Cont_get( pkt, from )
MPID_PKT_GET_T *pkt;
int            from;
{
MPIR_RHANDLE *dmpi_recv_handle;
int          err;
char         *address;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	    "[%d]Cont-get from %d (tag %d) offset %d\n", 
	    MPID_MyWorldRank, from, pkt->tag, pkt->cur_offset );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

if (pkt->recv_id == 0) {
    fprintf( stderr, "Internal error! null recv id\n" );
    exit(1);
    }
dmpi_recv_handle = (MPIR_RHANDLE *)(pkt->recv_id);
/* 
   add more data.  Note that if this is an "unexpected" message and we
   are doing aggressive delivery, then we need to use the temp field, not
   the start field.  Check to see if start is null or not.

   One of start and temp must be null, or the code will become confused.
 */
if (dmpi_recv_handle->dev_rhandle.start && 
    dmpi_recv_handle->dev_rhandle.temp) {
    fprintf( stderr, 
	    "[%d] WARNING: Internal error; msgs have both start and temp\n",
	    MPID_MyWorldRank );
    }
address = (char *)(dmpi_recv_handle->dev_rhandle.start);
if (!address) {
    DEBUG_PRINT_MSG("R Cont-get for unexpected receive")
    address = (char *)dmpi_recv_handle->dev_rhandle.temp;
    }
if (!address) {
    fprintf( stderr, "Internal error! Null buffer for receive data\n" );
    exit(1);
    }
err = MPID_SHMEM_Do_get_to_mem( address + pkt->cur_offset, from, pkt );
if (pkt->cur_offset >= pkt->len) {
    DMPI_mark_recv_completed(dmpi_recv_handle);
    }
return err;
}

/* 
   Send-side operations
 */
int MPID_SHMEM_post_send_long_get( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SEND_DECL(MPID_PKT_GET_T,pkt);
int              dest;
int              len_actual;

dest           = dmpi_send_handle->dest;
MPID_PKT_SEND_ALLOC(MPID_PKT_GET_T,pkt);
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_DO_GET);
MPID_PKT_SEND_SET(pkt,send_id,(MPID_Aint) dmpi_send_handle);
MPID_PKT_SEND_SET(pkt,recv_id,0);
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
len_actual     = len;
MPID_PKT_SEND_SET(pkt,address,
	  MPID_SetupGetAddress( mpid_send_handle->start, &len_actual, dest ));
MPID_PKT_SEND_SET(pkt,len_avail,len_actual);
MPID_PKT_SEND_SET(pkt,cur_offset,0);

DEBUG_PRINT_SEND_PKT("S Starting a send",pkt)
DEBUG_PRINT_LONG_MSG("S Sending extra-long message",pkt)
MPID_SENDCONTROL( mpid_send_handle, MPID_PKT_SEND_ADDR(pkt), 
		  sizeof(MPID_PKT_GET_T), dest );

/* Remember that we await a reply */
MPID_n_pending++;

MPID_PKT_SEND_FREE(pkt);
/* Message isn't completed until we receive the DONE_GET packet */
dmpi_send_handle->completer = MPID_CMPL_SEND_GET;

return MPI_SUCCESS;
}

int MPID_SHMEM_post_send_sync_long_get( dmpi_send_handle, mpid_send_handle, len ) 
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int len;
{
MPID_PKT_SEND_DECL(MPID_PKT_GET_T,pkt);
int              dest;
int              len_actual;

dest           = dmpi_send_handle->dest;
MPID_PKT_SEND_ALLOC(MPID_PKT_GET_T,pkt);
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_DO_GET_SYNC);
MPID_PKT_SEND_SET(pkt,send_id,(MPID_Aint) dmpi_send_handle);
MPID_PKT_SEND_SET(pkt,recv_id,0);
MPID_PKT_SEND_SET(pkt,context_id,dmpi_send_handle->contextid);
MPID_PKT_SEND_SET(pkt,lrank,dmpi_send_handle->lrank);
MPID_PKT_SEND_SET(pkt,tag,dmpi_send_handle->tag);
MPID_PKT_SEND_SET(pkt,len,len);
len_actual     = len;
MPID_PKT_SEND_SET(pkt,address,
	  MPID_SetupGetAddress( mpid_send_handle->start, &len_actual, dest ));
MPID_PKT_SEND_SET(pkt,len_avail,len_actual);
MPID_PKT_SEND_SET(pkt,cur_offset,0);
MPID_PKT_SEND_SET(pkt,sync_id,
		  MPID_SHMEM_Get_Sync_Id( dmpi_send_handle, mpid_send_handle ));

DEBUG_PRINT_SEND_PKT("S Starting a sync send",pkt)
DEBUG_PRINT_LONG_MSG("S Sending extra-long message",pkt)
MPID_SendControlBlock( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_GET_T), dest );

/* Remember that we await a reply */
MPID_n_pending++;

MPID_PKT_SEND_FREE(pkt);
/* Message isn't completed until we receive the sync ack packet */
dmpi_send_handle->completer = MPID_CMPL_SEND_GET;
return MPI_SUCCESS;
}

/* 
  Handle the ack for a Send/GET.  Mark the send as completed, and 
  free the get memory.
 */
int MPID_SHMEM_Done_get( pkt, from )
MPID_PKT_GET_T *pkt;
int            from;
{
MPIR_SHANDLE *dmpi_send_handle;

dmpi_send_handle = (MPIR_SHANDLE *)(pkt->send_id);
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
	    "[%d]Done-get from %d (tag = %d, left = %d)\n", 
	    MPID_MyWorldRank, from, pkt->tag, pkt->len - pkt->cur_offset );
    fflush( MPID_DEBUG_FILE );
    }
#endif                  /* #DEBUG_END# */

if (pkt->cur_offset < pkt->len) {
    /* A partial transmission.  Send it back */
    int m;
    MPID_PKT_SEND_DECL(MPID_PKT_GET_T,tpkt);

    m = pkt->len_avail;
    if (pkt->cur_offset + m > pkt->len) m = pkt->len - pkt->cur_offset;

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
    if (MPID_DebugFlag) {
	fprintf( MPID_DEBUG_FILE,
	    "[%d]Done-get returning %d bytes to %d\n", 
		MPID_MyWorldRank, m, from );
	fflush( MPID_DEBUG_FILE );
	}
#endif                  /* #DEBUG_END# */

    /* Now, get a new packet and send it back */
    MPID_PKT_SEND_ALLOC(MPID_PKT_GET_T,tpkt);
    MEMCPY( MPID_PKT_SEND_ADDR(tpkt), pkt, sizeof(MPID_PKT_GET_T) );
    MPID_PKT_SEND_SET(tpkt,len_avail,m);
    MPID_PKT_SEND_SET(tpkt,mode,MPID_PKT_CONT_GET);
    MEMCPY( MPID_PKT_SEND_GET(tpkt,address), 
	    ((char *)dmpi_send_handle->dev_shandle.start) + pkt->cur_offset, 
	    m );
    MPID_SendControl( MPID_PKT_SEND_ADDR(tpkt), sizeof(MPID_PKT_GET_T), from );
    MPID_PKT_SEND_FREE(tpkt);
    dmpi_send_handle->completer = MPID_CMPL_SEND_GET;
    }
else {
    /* Remember that we have finished this transaction */
    MPID_n_pending--;
#if defined(MPID_PKT_GET_NEEDS_ACK)
    MPID_FreeGetAddress( pkt->address );
    pkt->address = 0;
#endif
    DMPI_mark_send_completed( dmpi_send_handle );
    }
return MPI_SUCCESS;
}

/* Complete a get by continuing to process requests until the code marks
   the operation as completed */
int MPID_SHMEM_Cmpl_send_get( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
while (!MPID_Test_handle(dmpi_send_handle)) {
    (void)MPID_SHMEM_check_incoming( MPID_BLOCKING );
    }
return MPI_SUCCESS;
}

#endif
