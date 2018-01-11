 /*
 *  $Id: t3dsend.c,v 1.8 1995/09/15 02:00:59 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3dsend.c,v 1.8 1995/09/15 02:00:59 bright Exp $";
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
T3D_Buf_Status *t3d_dest_bufs;

/***************************************************************************
   T3D_Set_send_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;   /* Set to 0 for no debug output */
void T3D_Set_send_debug_flag( f )
int f;
{
    DebugFlag = f;
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
    T3D_PKT_LONG_SYNC_T *pkt;
    int             dest = dmpi_send_handle->dest;
    int             msglen;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    /* Wait for send buffer to be available */
    while( t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-got send buffer\n");
    }
#   endif

    /* Fill in the packet information */
        pkt                      = (T3D_PKT_LONG_SYNC_T *)&t3d_recv_bufs[t3d_myid];
        pkt->status             = T3D_BUF_IN_USE;
        pkt->mode                = T3D_PKT_LONG_SYNC;
        pkt->context_id          = dmpi_send_handle->contextid;
        pkt->lrank               = dmpi_send_handle->lrank;
        pkt->tag                 = dmpi_send_handle->tag;
        pkt->len                 = len;
        pkt->local_send_completed = (char *)&(dmpi_send_handle->completer);
        pkt->buffer     = (char *)&(mpid_send_handle->remote_long_buffer);

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("mpid_send_handle->remote_long_buffer    = 0x%x\n",
                    mpid_send_handle->remote_long_buffer);
        T3D_Printf("mpid_send_handle  remote_long_completed = 0x%x\n",
                    mpid_send_handle->remote_long_buffer + 1);
    }
#   endif

        /* We don't give it an ID */
        mpid_send_handle->sid = 0;

        msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_LONG_SYNC_T) );

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
 	T3D_Printf("  &rmt_buf   = 0x%x\n",pkt->buffer);
	T3D_Printf("  &rmt_comp  = 0x%x\n",pkt->buffer+1);
    }
#   endif

        /* Send the packet */
        shmem_put( (long *)&t3d_recv_bufs[T3D_MY_PE],
                   (long *)pkt,
                    msglen,
                    dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-long packet header sent\n");
    }
#   endif

    mpid_send_handle->header_sent = 1;
    mpid_send_handle->mode        = T3D_PKT_LONG_SYNC;

    T3D_Complete_send( dmpi_send_handle );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-waiting for completion by receiver\n");
    }
#   endif

    /* Wait for send completed flag to be set by receiver */
    while( dmpi_send_handle->completer )
      T3D_Check_incoming( MPID_NOTBLOCKING );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long_sync-send completed by receiver\n");
    }
#   endif

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
    int              dest = dmpi_send_handle->dest;
    int              msglen;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    /* Wait for send buffer to be available */
    while( t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-got send buffer\n");
    }
#   endif


    /* These references are ordered to match the order they appear in the 
       structure */
    pkt             = (T3D_PKT_SHORT_SYNC_T *)&t3d_recv_bufs[t3d_myid];
    pkt->status     = T3D_BUF_IN_USE;
    pkt->mode       = T3D_PKT_SHORT_SYNC; 
    pkt->context_id = dmpi_send_handle->contextid;
    pkt->lrank      = dmpi_send_handle->lrank;
    pkt->tag        = dmpi_send_handle->tag;
    pkt->len        = len;
    pkt->local_send_completed = (char *)&(dmpi_send_handle->completer);

    /* Copy the message to the send space */
    memcpy( pkt->buffer, mpid_send_handle->start, len );

    /* We don't give it an ID */
    mpid_send_handle->sid = 0;

    msglen = T3D_MSG_LEN_64( (sizeof(T3D_PKT_HEAD_T) + len) );

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
    }
#   endif

    /* Send the packet */
    shmem_put( (long *)&t3d_recv_bufs[T3D_MY_PE], 
               (long *)pkt,
                msglen,
                dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-short packet sent\n");
    }
#   endif

    mpid_send_handle->header_sent = 1;
    mpid_send_handle->mode = T3D_PKT_SHORT_SYNC;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-waiting for completion by receiver\n");
    }
#   endif

    /* Wait for send completed flag to be set by receiver */
    while( dmpi_send_handle->completer )
      T3D_Check_incoming( MPID_NOTBLOCKING );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short_sync-send completed by receiver\n");
    }
#   endif


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

    mpid_send_handle->local_completed = (char *)&(dmpi_send_handle->completer);

/* Is this possible ?? 
    if (dest == T3D_MY_PE) 
        return(T3D_Post_send_local(dmpi_send_handle, mpid_send_handle,
                                   len));
*/

    if ( len > T3D_BUFFER_LENGTH )
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

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    /* Wait for send buffer to be available */
    while( t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
      T3D_Check_incoming( MPID_NOTBLOCKING );

    t3d_dest_bufs[dest] = T3D_BUF_IN_USE;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-got send buffer\n");
    }
#   endif

    /* Fill in the packet information */
        pkt                      = (T3D_PKT_LONG_T *)&t3d_recv_bufs[t3d_myid];
        pkt->status             = T3D_BUF_IN_USE;
        pkt->mode                = T3D_PKT_LONG;
        pkt->context_id          = dmpi_send_handle->contextid;
        pkt->lrank               = dmpi_send_handle->lrank;
        pkt->tag                 = dmpi_send_handle->tag;
        pkt->len                 = len;
        pkt->buffer     = (char *)&(mpid_send_handle->remote_long_buffer);

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("mpid_send_handle->remote_long_buffer    = 0x%x\n",
                    mpid_send_handle->remote_long_buffer);
        T3D_Printf("mpid_send_handle  remote_long_completed = 0x%x\n",
                    mpid_send_handle->remote_long_buffer + 1);
    }
#   endif

        /* We don't give it an ID */
        mpid_send_handle->sid = 0;

        msglen = T3D_MSG_LEN_64( sizeof(T3D_PKT_LONG_T) );

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
 	T3D_Printf("  &rmt_buf   = 0x%x\n",pkt->buffer);
	T3D_Printf("  &rmt_comp  = 0x%x\n",pkt->buffer+1);
    }
#   endif

        /* Send the packet */
        shmem_put( (long *)&t3d_recv_bufs[T3D_MY_PE],
                   (long *)pkt,
                    msglen,
                    dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-long packet header sent\n");
    }
#   endif

    mpid_send_handle->header_sent = 1;

    T3D_Complete_send( dmpi_send_handle );

    DMPI_mark_send_completed( dmpi_send_handle );

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

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_long-waiting for send buffer #%d at 0x%x\n",
                    dest, &t3d_dest_bufs[dest] );
    }
#   endif

    /* Wait for send buffer to be available */
    while( t3d_dest_bufs[dest] != T3D_BUF_AVAIL )
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
    pkt->status     = T3D_BUF_IN_USE;
    pkt->mode       = T3D_PKT_SHORT; 
    pkt->context_id = dmpi_send_handle->contextid;
    pkt->lrank      = dmpi_send_handle->lrank;
    pkt->tag        = dmpi_send_handle->tag;
    pkt->len        = len;

    /* Copy the message to the send space */
    memcpy( pkt->buffer, mpid_send_handle->start, len );

    /* We don't give it an ID */
    mpid_send_handle->sid = 0;

    msglen = T3D_MSG_LEN_64( (sizeof(T3D_PKT_HEAD_T) + len) );

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
    }
#   endif

    /* Send the packet */
    shmem_put( (long *)&t3d_recv_bufs[T3D_MY_PE], 
               (long *)pkt,
                msglen,
                dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_short-short packet sent\n");
    }
#   endif

    mpid_send_handle->header_sent = 1;

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
    int              is_posted;
    int              err = MPI_SUCCESS;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send_local\n");
    }
#   endif

    DMPI_msg_arrived( dmpi_send_handle->lrank, dmpi_send_handle->tag, 
                      dmpi_send_handle->contextid, 
                      &dmpi_recv_handle, &is_posted );

    if (is_posted) {

        dmpi_recv_handle->totallen = len;
        
        /* Copy message if needed and mark the receive as completed */
        if (len > 0) 
            memcpy( dmpi_recv_handle->dev_rhandle.start, 
                    dmpi_send_handle->dev_shandle.start,
                    len ); 
        DMPI_mark_recv_completed(dmpi_recv_handle);

	/* Mark the send as completed. */
	DMPI_mark_send_completed( dmpi_send_handle );

        return (err);
    }
    else {

        MPID_RHANDLE *mpid_recv_handle;
        char         *address;
        
        /* initialize mpid handle */
        mpid_recv_handle                  = &dmpi_recv_handle->dev_rhandle;
        mpid_recv_handle->bytes_as_contig = len;
        mpid_recv_handle->mode            = 0;   
        mpid_recv_handle->from            = T3D_UNDEFINED;
        
        /* copy the message */
        if (len > 0) {
            mpid_recv_handle->temp = (char *)T3D_MALLOC(len);
            if ( ! mpid_recv_handle->temp )
                T3D_Error("Out of memory for unexpected messages\n");
            memcpy( mpid_recv_handle->temp, 
                    dmpi_send_handle->dev_shandle.start, 
                    len );
        }
        mpid_send_handle->header_sent = 1;
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
    MPID_SHANDLE *mpid_send_handle;
    int           len;
    int           dest;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_send\n");
    }
#   endif

    mpid_send_handle = &dmpi_send_handle->dev_shandle;
    len              = mpid_send_handle->bytes_as_contig;

    dest = dmpi_send_handle->dest;

    /* Message to me */
    if (dest == T3D_MY_PE) 
        return(T3D_Post_send_local(dmpi_send_handle, mpid_send_handle,
                                   len));

    /* Handle the cases for various length messages */
    if ( len > T3D_BUFFER_LENGTH )
      return(T3D_Post_send_long(dmpi_send_handle, mpid_send_handle, len));
    else
      return(T3D_Post_send_short(dmpi_send_handle, mpid_send_handle, len));

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

    T3D_Complete_send( dmpi_send_handle );

    DMPI_mark_send_completed( dmpi_send_handle );

    return (MPI_SUCCESS);
}


/***************************************************************************
   T3D_Complete_send
 ***************************************************************************/
int T3D_Complete_send( dmpi_send_handle ) 
MPIR_SHANDLE *dmpi_send_handle;
{
    MPID_SHANDLE *mpid_send_handle = &(dmpi_send_handle->dev_shandle);
    int           completed        = 0;
    char         *target,*stackptr,*heapptr;
    int           numbytes,msglen;


#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Complete_send\n");
    }
#   endif
  
    /* Short message needs no completion */
    if ( mpid_send_handle->bytes_as_contig < T3D_BUFFER_LENGTH ) {
#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
      if (DebugFlag) {
        T3D_Printf("T3D_Complete_send-short buffer needs no completing\n");
      }
#   endif
      return MPI_SUCCESS;
    }
    
    /* Long message might need to be completed */
    if ( dmpi_send_handle->completer ) {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
      if (DebugFlag) {
        T3D_Printf("T3D_Complete_send-long buffer needs completing\n");
      }
#   endif
        
      /* If send needs to be completed, wait for pointer to remote buffer */
      while ( (mpid_send_handle->remote_long_buffer    == (char *)NULL) ||
              (mpid_send_handle->remote_long_completed == (char *)NULL)    )
        T3D_Check_incoming( MPID_NOTBLOCKING );
     
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("sending long buffer to 0x%x\n",mpid_send_handle->remote_long_buffer);
    }
#   endif

      stackptr = (char *)&msglen;
      heapptr  = (char *)t3d_heap_limit;
      target   = mpid_send_handle->remote_long_buffer;
      numbytes = mpid_send_handle->bytes_as_contig;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Complete_send - stack    = 0x%x\n",stackptr);
	T3D_Printf("                    heap     = 0x%x\n",heapptr );
	T3D_Printf("                    target   = 0x%x\n",target  );
	T3D_Printf("                    bytes    = %d\n",numbytes  );
    }
#   endif

      if ( ( (target-numbytes) < stackptr ) && ( (target+numbytes) > heapptr  ) ) {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
         if (DebugFlag) {
	   T3D_Printf("T3D_Complete_send-detected invalid target address\n");
	 }
#   endif
	 /* check target to see if it is less than the stack and greater than the heap */
	 if ( (stackptr - (target-numbytes) ) < ((target+numbytes) - heapptr) ) {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	   if (DebugFlag) {
	   }
#   endif
           /* stack variable, so extend the stack */
           shmem_stack( (void *)(target - numbytes) );
	 }
	 else {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
	   if (DebugFlag) {
	     T3D_Printf("T3D_Complete_send-extending heap\n");
	   }
#   endif

	   /* heap variable, so extend the heap and get new limit */
	   malloc_brk( (char *)target + numbytes );
	   t3d_heap_limit = sbreak( 0 ); 
	 }
       }
      
      /* Send into remote long buffer */
      T3D_LONG_SEND( mpid_send_handle->remote_long_buffer,
		     mpid_send_handle->start,
		     mpid_send_handle->bytes_as_contig,
		     dmpi_send_handle->dest );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("sending completed to 0x%x\n",mpid_send_handle->remote_long_completed);
    }
#   endif

      /* Notify remote receive handle of send completion */
      shmem_put( (long *)mpid_send_handle->remote_long_completed,
                 (long *)&completed,
                  1,
                  dmpi_send_handle->dest );


    }

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
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_SEND)
    if (DebugFlag) {
        T3D_Printf("T3D_Test_send\n");
    }
#   endif

    return ( dmpi_send_handle->completer == 0 );
}

