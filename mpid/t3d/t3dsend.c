/*
 *  $Id: t3dsend.c,v 1.9 1995/09/15 19:19:30 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3dsend.c,v 1.9 1995/09/15 19:19:30 bright Exp $";
#endif

#include "mpid.h"
#include <malloc.h>
#include <unistd.h>


/* 
 * ADI message send routines
 *
 *
 * Interface Description
 * ---------------------
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *   void T3D_Set_send_debug_flag( int f )
 *      Sets the debug flag for runtime send debugging.
 *
 *   int T3D_Post_send( MPIR_SHANDLE *dmpi_send_handle ) 
 *      Post a standard send.
 *
 *   int T3D_Post_send_sync( MPIR_SHANDLE *dmpi_send_handle ) 
 *      Post a synchronous send.
 *
 *   int T3D_Post_send_ready( MPIR_SHANDLE *dmpi_send_handle ) 
 *      Post a ready send.
 *
 *   int T3D_Blocking_send( MPIR_SHANDLE *dmpi_send_handle )
 *      A blocking send.
 *
 *   int T3D_Complete_send( MPIR_SHANDLE *dmpi_send_handle ) 
 *      Complete a send operation.
 *
 *   int T3D_Test_send( MPIR_SHANDLE *dmpi_send_handle )
 *      Test for the completion of a send.
 *
 *
 */

/****************************************************************************
  Global variables
 ***************************************************************************/
#pragma _CRI cache_align  t3d_dest_bufs
/* #pragma _CRI suppress     t3d_dest_bufs */
volatile T3D_Buf_Status  *t3d_dest_bufs;
extern MPIR_QHDR T3D_long_sends;

/***************************************************************************
   T3D_Set_send_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;   /* Set to 0 for no debug output */
void T3D_Set_send_debug_flag( f )
int f;
{
    DebugFlag = f;
}

/* FORTRAN equivalent */
void T3D_SET_SEND_DEBUG_FLAG()
{
    DebugFlag = 1;
}

/****************************************************************************
  T3D_Post_send_long_sync
 ***************************************************************************/
int T3D_Post_send_long_sync( dmpi_send_handle, mpid_send_handle, len )
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int           len;
{
    /* Send a long message */
    T3D_PKT_LONG_SYNC_T *pkt,*dummy=0;
    int                  dest = dmpi_send_handle->dest;
    int                  msglen;
    char                *target;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    mpid_send_handle->mode        = T3D_PKT_LONG_SYNC;

    /* Wait for send buffer to be available */
    while( (volatile)t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-got send buffer\n");
    }
#   endif

    /* Fill in the packet information */
    pkt                        = (T3D_PKT_LONG_SYNC_T *)&t3d_recv_bufs[t3d_myid];
    pkt->mode                  = T3D_PKT_LONG_SYNC;
    pkt->context_id            = dmpi_send_handle->contextid;
    pkt->lrank                 = dmpi_send_handle->lrank;
    pkt->tag                   = dmpi_send_handle->tag;
    pkt->len                   = len;
    pkt->status                = T3D_BUF_IN_USE;
    pkt->long_send_info_target = &mpid_send_handle->long_send_info;
    pkt->local_send_completed  = (char *)&(dmpi_send_handle->completer);

    msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_HEAD_SIZE_T) );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-sending long packet header\n");
	T3D_Printf("message contents\n");
	T3D_Printf("  to         = %d\n",dest);
	T3D_Printf("  status     = %d\n",pkt->status);
	T3D_Printf("  mode       = %d\n",pkt->mode);
	T3D_Printf("  context id = %d\n",pkt->context_id);
	T3D_Printf("  lrank      = %d\n",pkt->lrank);
	T3D_Printf("  tag        = %d\n",pkt->tag);
	T3D_Printf("  length     = %d\n",pkt->len);
	T3D_Printf("  completed  = 0x%x\n",pkt->local_send_completed);
	T3D_Printf("  lsit       = 0x%x\n",pkt->long_send_info_target);
    }
#   endif

        /* Send the packet */
    shmem_put((long *)&(pkt->local_send_completed),
	      (long *)&(pkt->local_send_completed),
	      msglen,
	      dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-long packet header sent\n");
    }
#   endif

    /* Queue up the long send request */
    mpid_send_handle->p =
      T3D_enqueue( &T3D_long_sends, (MPIR_COMMON *)dmpi_send_handle, MPIR_QSHANDLE );

    return (MPI_SUCCESS);
    
} /* T3D_Post_send_long_sync */


/****************************************************************************
  T3D_Post_send_short_sync

 ***************************************************************************/
int T3D_Post_send_short_sync( dmpi_send_handle, mpid_send_handle, len )
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int           len;
{
    /* Send a short message */
    T3D_PKT_SHORT_SYNC_T *pkt;
    int                   dest = dmpi_send_handle->dest;
    int                   msglen;
    int                   buf_loc;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    mpid_send_handle->mode = T3D_PKT_SHORT_SYNC;

    /* Wait for send buffer to be available */
    while( (volatile)t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-got send buffer\n");
    }
#   endif


    /* These references are ordered to match the order they appear in the 
       structure */
    pkt                       = (T3D_PKT_SHORT_SYNC_T *)&t3d_recv_bufs[t3d_myid];
    pkt->mode                 = T3D_PKT_SHORT_SYNC; 
    pkt->context_id           = dmpi_send_handle->contextid;
    pkt->lrank                = dmpi_send_handle->lrank;
    pkt->tag                  = dmpi_send_handle->tag;
    pkt->len                  = len;
    pkt->status               = T3D_BUF_IN_USE;
    pkt->local_send_completed = (char *)&(dmpi_send_handle->completer);

    if (0 < len) {
      buf_loc = T3D_BUFFER_LENGTH - T3D_MSG_LEN_32(len) * 4;
      T3D_MEMCPY(&(pkt->buffer[buf_loc]),mpid_send_handle->start,len);
      msglen = T3D_MSG_LEN_32(sizeof(T3D_PKT_HEAD_SIZE_T)) + T3D_MSG_LEN_32(len);
      shmem_put32((long *)&(pkt->buffer[buf_loc]),
		  (long *)&(pkt->buffer[buf_loc]),
		  msglen,
		  dest );
    }
    else {
      msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_HEAD_SIZE_T) );
      shmem_put((long*)&(pkt->local_send_completed),
		(long*)&(pkt->local_send_completed),
		msglen,
		dest);
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-sending short packet\n");
	T3D_Printf("message contents\n");
	T3D_Printf("  to         = %d\n",dest);
	T3D_Printf("  status     = %d\n",pkt->status);
	T3D_Printf("  mode       = %d\n",pkt->mode);
	T3D_Printf("  context id = %d\n",pkt->context_id);
	T3D_Printf("  lrank      = %d\n",pkt->lrank);
	T3D_Printf("  tag        = %d\n",pkt->tag);
	T3D_Printf("  length     = %d\n",pkt->len);
	T3D_Printf("  completed  = 0x%x\n",pkt->local_send_completed);
        T3D_Printf("T3D_Post_send_short_sync-short packet sent\n");
        T3D_Printf("T3D_Post_send_short_sync-waiting for completion by receiver\n");
    }
#   endif

    mpid_send_handle->body_sent = 1;

    return (MPI_SUCCESS);

} /* T3D_Post_send_short_sync */

/***************************************************************************
   T3D_Post_send_sync
 ***************************************************************************/
int T3D_Post_send_sync( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
    MPID_SHANDLE *mpid_send_handle;
    int           len;
    int           dest;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_sync\n");
    }
#   endif

    mpid_send_handle = &dmpi_send_handle->dev_shandle;
    len              = mpid_send_handle->bytes_as_contig;
    dest = dmpi_send_handle->dest;

    if ( len >= T3D_BUFFER_LENGTH )
      return(T3D_Post_send_long_sync(dmpi_send_handle, mpid_send_handle, len));
    else
      return(T3D_Post_send_short_sync(dmpi_send_handle, mpid_send_handle, len));

    return (MPI_SUCCESS);

}


/***************************************************************************
   T3D_Post_send_ready
 ***************************************************************************/
int T3D_Post_send_ready( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("Ready mode not yet supported.  Using Regular send.\n");
    }
#   endif

    return (MPI_SUCCESS);
}


/****************************************************************************
  T3D_Post_send_long
 ***************************************************************************/
int T3D_Post_send_long( dmpi_send_handle, mpid_send_handle, len )
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int           len;
{
    /* Send a long message */
    T3D_PKT_LONG_T *pkt;
    int             dest = dmpi_send_handle->dest;
    int             msglen;
    char           *target;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    mpid_send_handle->mode = T3D_PKT_LONG;

    /* Wait for send buffer to be available */
    while( (volatile)t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-got send buffer\n");
    }
#   endif

    /* Fill in the packet information */
    pkt                        = (T3D_PKT_LONG_T *)&t3d_recv_bufs[t3d_myid];
    pkt->mode                  = T3D_PKT_LONG;
    pkt->context_id            = dmpi_send_handle->contextid;
    pkt->lrank                 = dmpi_send_handle->lrank;
    pkt->tag                   = dmpi_send_handle->tag;
    pkt->len                   = len;
    pkt->status                = T3D_BUF_IN_USE;
    pkt->long_send_info_target = &mpid_send_handle->long_send_info;

    msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_HEAD_SIZE_T) ) - 1;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-sending long packet header\n");
	T3D_Printf("message contents\n");
	T3D_Printf("  to         = %d\n",dest);
	T3D_Printf("  status     = %d\n",pkt->status);
	T3D_Printf("  mode       = %d\n",pkt->mode);
	T3D_Printf("  context id = %d\n",pkt->context_id);
	T3D_Printf("  lrank      = %d\n",pkt->lrank);
	T3D_Printf("  tag        = %d\n",pkt->tag);
	T3D_Printf("  length     = %d\n",pkt->len);
	T3D_Printf("  lsit       = 0x%x\n",pkt->long_send_info_target);
    }
#   endif

    /* Send the packet */
    shmem_put((long *)&(pkt->long_send_info_target),
	      (long *)&(pkt->long_send_info_target),
	      msglen,
	      dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-long packet header sent\n");
    }
#   endif

    /* Queue up the long send request */
    mpid_send_handle->p =
      T3D_enqueue( &T3D_long_sends, (MPIR_COMMON *)dmpi_send_handle, MPIR_QSHANDLE );

    return (MPI_SUCCESS);
    
} /* T3D_Post_send_long */


/****************************************************************************
  T3D_Post_send_short

 ***************************************************************************/
int T3D_Post_send_short( dmpi_send_handle, mpid_send_handle, len )
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int           len;
{
    /* Send a short message */
    T3D_PKT_SHORT_T *pkt;
    int              dest = dmpi_send_handle->dest;
    int              msglen;
    int              buf_loc;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    mpid_send_handle->mode = T3D_PKT_SHORT;

    /* Wait for send buffer to be available */
    while( (volatile)t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short-got send buffer\n");
    }
#   endif

    /* These references are ordered to match the order they appear in the 
       structure */
    pkt             = (T3D_PKT_SHORT_T *)&t3d_recv_bufs[t3d_myid];
    pkt->mode       = T3D_PKT_SHORT; 
    pkt->context_id = dmpi_send_handle->contextid;
    pkt->lrank      = dmpi_send_handle->lrank;
    pkt->tag        = dmpi_send_handle->tag;
    pkt->len        = len;
    pkt->status     = T3D_BUF_IN_USE;

    if (0 < len) {
      buf_loc = T3D_BUFFER_LENGTH - 4*T3D_MSG_LEN_32( len ); 
      msglen = T3D_MSG_LEN_32(sizeof(T3D_PKT_HEAD_SIZE_T)) + T3D_MSG_LEN_32(len);
      T3D_MEMCPY(&(pkt->buffer[buf_loc]),mpid_send_handle->start,len);
      shmem_put32((long *)&(pkt->buffer[buf_loc]),
		  (long *)&(pkt->buffer[buf_loc]),
		  msglen,
		  dest );
    }
    else {
      msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_HEAD_SIZE_T) ) - 2;
      shmem_put((long*)&(pkt->mode),
	        (long*)&(pkt->mode),
	        msglen,
	        dest);
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short-sending short packet\n");
	T3D_Printf("message contents\n");
	T3D_Printf("  to         = %d\n",dest);
	T3D_Printf("  status     = %d\n",pkt->status);
	T3D_Printf("  mode       = %d\n",pkt->mode);
	T3D_Printf("  context id = %d\n",pkt->context_id);
	T3D_Printf("  lrank      = %d\n",pkt->lrank);
	T3D_Printf("  tag        = %d\n",pkt->tag);
	T3D_Printf("  length     = %d\n",pkt->len);
        T3D_Printf("T3D_Post_send_short-short packet sent\n");
    }
#   endif

    mpid_send_handle->body_sent = 1;

    /* Mark the send as completed. */
    DMPI_mark_send_completed( dmpi_send_handle );

    return (MPI_SUCCESS);

} /* T3D_Post_send_short */


/****************************************************************************
  T3D_Post_send_local

  Description
    The T3D API does not support sending a message to yourself.  
    This function notifies the soft layer that a message has arrived,
    then copies the body of the message the dmpi handle.  Currently,
    we post (copy) the sent message directly to the unexpected message
    queue or the expected message queue.
v    
 ***************************************************************************/
int T3D_Post_send_local( dmpi_send_handle, mpid_send_handle, len )
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;
int           len;
{
    MPIR_RHANDLE    *dmpi_recv_handle;
    MPID_RHANDLE    *mpid_recv_handle;
    int              is_posted;
    int              err = MPI_SUCCESS;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_local\n");
    }
#   endif

    DMPI_msg_arrived( t3d_myid, dmpi_send_handle->tag, 
                      dmpi_send_handle->contextid, 
                      &dmpi_recv_handle, &is_posted );

    if (is_posted) {

      dmpi_recv_handle->source   = dmpi_send_handle->lrank;
      dmpi_recv_handle->tag      = dmpi_send_handle->tag;
      dmpi_recv_handle->totallen = len;
        
      /* Copy message if needed and mark the receive as completed */
      if (len > 0) 
	T3D_MEMCPY( dmpi_recv_handle->dev_rhandle.start, 
		    dmpi_send_handle->dev_shandle.start,
		    len ); 
      
      DMPI_mark_recv_completed(dmpi_recv_handle);

      /* Mark the send as completed. */
      DMPI_mark_send_completed( dmpi_send_handle );

      return (err);
    }
    else {
        
        /* initialize mpid handle */
        mpid_recv_handle                  = &dmpi_recv_handle->dev_rhandle;
        mpid_recv_handle->from            = t3d_myid;
        mpid_recv_handle->mode            = 0;   
        mpid_recv_handle->bytes_as_contig = len;
        
        /* copy the message */
        if (len > 0) {
         if ( (mpid_recv_handle->temp = (void *)T3D_MALLOC(len)) !=
	   (void *)NULL ) { 
	   T3D_MEMCPY( mpid_recv_handle->temp,
	               dmpi_send_handle->dev_shandle.start, 
		       len );
	}
	 else 
	   T3D_Error("Out of memory for unexpected messages\n");
        }

        DMPI_mark_recv_completed(dmpi_recv_handle);

	/* Mark the send as completed. */
	DMPI_mark_send_completed( dmpi_send_handle );

        return (err);
    }

    /* should never get here */
    return (MPI_ERR_UNKNOWN);

} /* T3D_Post_send_local */

 
/***************************************************************************
   T3D_Post_send
 ***************************************************************************/
int T3D_Post_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
    MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send\n");
    }
#   endif

    /* Message to me */
    if ( dmpi_send_handle->dest == T3D_MY_PE ) 
        return(T3D_Post_send_local(dmpi_send_handle, mpid_send_handle,
                                   mpid_send_handle->bytes_as_contig));

    /* Handle the cases for various length messages */
    if ( mpid_send_handle->bytes_as_contig >= T3D_BUFFER_LENGTH )
      return(T3D_Post_send_long(dmpi_send_handle, mpid_send_handle, 
				mpid_send_handle->bytes_as_contig));
    else
      return(T3D_Post_send_short(dmpi_send_handle, mpid_send_handle, 
				 mpid_send_handle->bytes_as_contig));

} /* T3D_Post_send */


/***************************************************************************
   T3D_Blocking_send
 ***************************************************************************/
int T3D_Blocking_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{
    int stat;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Blocking_send\n");
    }
#   endif

    /* Post the send and wait for its completion */
    if ((stat = T3D_Post_send( dmpi_send_handle )) != MPI_SUCCESS)
        return (stat);

    if ( dmpi_send_handle->completer != 0 ) {
       T3D_Complete_send( dmpi_send_handle );
    }

    return (MPI_SUCCESS);
}


/***************************************************************************
   T3D_Complete_send
 ***************************************************************************/
int T3D_Complete_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
    MPID_SHANDLE *mpid_send_handle = &(dmpi_send_handle->dev_shandle);
    void         *tmp_start;
    int           msglen;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Complete_send\n");
    }
#   endif

    if ( ( dmpi_send_handle->completer != 0 ) && ( ! mpid_send_handle->body_sent ) ) {

      if ( ((mpid_send_handle->mode == T3D_PKT_LONG)     ||
	    (mpid_send_handle->mode == T3D_PKT_LONG_SYNC)   ) &&
	    (mpid_send_handle->body_sent == 0)                   ) {

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	if (DebugFlag) {
	  T3D_Printf("   Waiting for long send info\n");
	}
#       endif

	while ( (volatile)mpid_send_handle->long_send_info.length == -1 )
	  T3D_Check_incoming( MPID_NOTBLOCKING );
	
#        if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	if (DebugFlag) {
	  T3D_Printf("   long send info =\n");
	  T3D_Printf("     target buffer    = 0x%x\n",
		     mpid_send_handle->long_send_info.target_buffer );
	  T3D_Printf("     target completer = 0x%x\n",
		     mpid_send_handle->long_send_info.target_completer );
	  T3D_Printf("     completer value  = %d\n",
		     mpid_send_handle->long_send_info.completer_value );
	  T3D_Printf("     length           = %d\n",
		     mpid_send_handle->long_send_info.length        );
	}
#       endif


	if ( mpid_send_handle->long_send_info.length > 0 ) {

	  if ( IS_4BYTE_ALIGNED(mpid_send_handle->start) ) {
	    tmp_start = mpid_send_handle->start;
	  }
	  else {
	    if ( ( tmp_start = malloc(mpid_send_handle->long_send_info.length) ) ==
		 (void *)NULL ) {
	      T3D_Error("malloc() failed\n");
	      T3D_Abort(1);
	    }
	    memcpy( tmp_start,
		    mpid_send_handle->start,
		    mpid_send_handle->long_send_info.length );
	  }

	  T3D_CHECK_TARGET( mpid_send_handle->long_send_info.target_buffer,
			    mpid_send_handle->long_send_info.length );


          msglen = T3D_MSG_LEN_32(mpid_send_handle->long_send_info.length);

	  shmem_put32( (short *)mpid_send_handle->long_send_info.target_buffer,
		       (short *)tmp_start,
		       msglen,
		       dmpi_send_handle->dest );

	  T3D_RESET_STACK;

	  if ( tmp_start != mpid_send_handle->start )
	    T3D_FREE( tmp_start );

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	if (DebugFlag) {
	  T3D_Printf("   msglen = %d\n",msglen);
	  T3D_Printf("   Sending completed flag\n");
	}
#       endif

	T3D_CHECK_TARGET( mpid_send_handle->long_send_info.target_completer, 8 );

	shmem_put( (long *)mpid_send_handle->long_send_info.target_completer,
		   (long *)&mpid_send_handle->long_send_info.completer_value,
		   1,
		   dmpi_send_handle->dest );

	T3D_RESET_STACK;

	}

	mpid_send_handle->body_sent = 1;

	if ( mpid_send_handle->mode == T3D_PKT_LONG ) {
	    DMPI_mark_send_completed( dmpi_send_handle );
	    T3D_dequeue( &T3D_long_sends, mpid_send_handle->p ); 
	}
      }
      else {
	DMPI_mark_send_completed( dmpi_send_handle );
	T3D_dequeue( &T3D_long_sends, mpid_send_handle->p ); 
      }



#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
      if (DebugFlag) {
	  T3D_Printf("   Waiting for send completed at 0x%x\n",
		      &dmpi_send_handle->completer);
      }
#     endif

    }

      /* wait for synchronous reply */
      while( (volatile)dmpi_send_handle->completer > 0 )
	T3D_Check_incoming( MPID_NOTBLOCKING );


#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("   Send is complete\n");
    }
#   endif


    return (MPI_SUCCESS);
      
}


/***************************************************************************
   T3D_Test_send

   Description:
      This routine tests for a send to be completed.
 ***************************************************************************/
int T3D_Test_send( dmpi_send_handle )
MPIR_SHANDLE *dmpi_send_handle;
{

    MPID_SHANDLE *mpid_send_handle = &dmpi_send_handle->dev_shandle;
    void         *tmp_start;
    int           msglen;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Test_send\n");
    }
#   endif

    if ( (dmpi_send_handle->completer != 0) && (! mpid_send_handle->body_sent) ) {

      if ( ((mpid_send_handle->mode == T3D_PKT_LONG)     ||
	    (mpid_send_handle->mode == T3D_PKT_LONG_SYNC)   ) &&
	   ((mpid_send_handle->body_sent == 0)            &&
	    (mpid_send_handle->long_send_info.length != -1) ) ) {

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	if (DebugFlag) {
	  T3D_Printf("   long send info =\n");
	  T3D_Printf("     target buffer    = 0x%x\n",
		     mpid_send_handle->long_send_info.target_buffer );
	  T3D_Printf("     target completer = 0x%x\n",
		     mpid_send_handle->long_send_info.target_completer );
	  T3D_Printf("     completer value  = %d\n",
		     mpid_send_handle->long_send_info.completer_value );
	  T3D_Printf("     length           = %d\n",  
		     mpid_send_handle->long_send_info.length        );
	}
#       endif

	if ( mpid_send_handle->long_send_info.length > 0 ) {

	  if ( IS_4BYTE_ALIGNED(mpid_send_handle->start) ) {
	    tmp_start = mpid_send_handle->start;
	  }
	  else {
	    if ( ( tmp_start = malloc(mpid_send_handle->long_send_info.length) ) ==
		 (void *)NULL ) {
	      T3D_Error("malloc() failed\n");
	      T3D_Abort(1);
	    }
	    memcpy( tmp_start,
		    mpid_send_handle->start,
		    mpid_send_handle->long_send_info.length );
	  }

	  T3D_CHECK_TARGET( mpid_send_handle->long_send_info.target_buffer,
			    mpid_send_handle->long_send_info.length );
	 
	  msglen = T3D_MSG_LEN_32(mpid_send_handle->long_send_info.length);

	  shmem_put32( (short *)mpid_send_handle->long_send_info.target_buffer,
		       (short *)mpid_send_handle->start,
		       msglen,
		       dmpi_send_handle->dest );

	  T3D_RESET_STACK;

	  if ( tmp_start != mpid_send_handle->start )
	    T3D_FREE( tmp_start );

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	  if (DebugFlag) {
	    T3D_Printf("   msglen = %d\n",msglen);
	    T3D_Printf("   Sending completed flag to 0x%x\n",
		        mpid_send_handle->long_send_info.target_completer);
	  }
#         endif

	  T3D_CHECK_TARGET( mpid_send_handle->long_send_info.target_completer, 8 );
	  
	  shmem_put( (long *)mpid_send_handle->long_send_info.target_completer,
		     (long *)&mpid_send_handle->long_send_info.completer_value,
		     1,
		     dmpi_send_handle->dest );

	  T3D_RESET_STACK;

	  mpid_send_handle->body_sent = 1;

	  if ( mpid_send_handle->mode == T3D_PKT_LONG ) {
	    DMPI_mark_send_completed( dmpi_send_handle );
	    T3D_dequeue( &T3D_long_sends, mpid_send_handle->p );
	  }
        }	
	else {
	  DMPI_mark_send_completed( dmpi_send_handle );
	  T3D_dequeue( &T3D_long_sends, mpid_send_handle->p );
        }	
	  
      } 

    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
      if ( dmpi_send_handle->completer == 0 )
	T3D_Printf("  Send is complete\n");
      else 
	T3D_Printf("  Send is not complete\n");
      }
#   endif

    return ( dmpi_send_handle->completer == 0 );

}

/***************************************************************************
   T3D_Process_long_sends

   Description:

 ***************************************************************************/
void T3D_Process_long_sends()
{
  MPIR_SHANDLE *dmpi_send_handle;
  MPIR_QHDR    *queue = &T3D_long_sends;
  MPIR_QEL     *p;

# if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
  if (DebugFlag) {
    T3D_Printf("T3D_Process_long_sends()\n");
  }
# endif

  p = queue->first;

  if (p) { 
    /* get the next send handle */
    dmpi_send_handle = ( MPIR_SHANDLE * ) p->ptr;

    /* try to complete it */
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
      T3D_Printf("   Calling T3D_Test_send()\n");
    }
#   endif

    T3D_Test_send( dmpi_send_handle );

  }
}

/* borrowed from MPIR_enqueue() */
MPIR_QEL *T3D_enqueue(header, object, object_type)
MPIR_QHDR     *header;
MPIR_COMMON   *object;
MPIR_QEL_TYPE object_type;
{

    MPIR_QEL      *p;

    header->currlen++;
    if (header->currlen > header->maxlen)
	header->maxlen++;

    p          = (MPIR_QEL *) MPIR_SBalloc( MPIR_qels );
    if (!p) {
	fprintf( stderr, "Out of memory !\n" );
	MPI_Abort( MPI_COMM_WORLD, 1 );
    }
    p->ptr      = (void *)object;
    p->qel_type = object_type;

    if (header->first == NULL)
    {
	header->first	    = header->last = p;
	p->prev		    = p->next      = NULL;
    }
    else
    {
	p->prev            = header->last;
	p->next            = NULL;
	header->last->next = p;
	header->last       = p;
    }
return p; 
}

/* borrowed from MPIR_dequeue() */
void T3D_dequeue(header, p)
MPIR_QHDR *header;
MPIR_QEL  *p;
{

    if ( p->next != NULL )
      p->next->prev = p->prev;
    else
      header->last  = p->prev;

    if ( p->prev != NULL )
      p->prev->next = p->next;
    else
      header->first = p->next;
    header->currlen--;
    MPIR_SBfree( MPIR_qels, p );/* free queue element */
  	    
}

