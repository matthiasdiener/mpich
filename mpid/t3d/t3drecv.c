/*
 *  $Id: t3drecv.c,v 1.6 1995/06/07 06:45:58 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3drecv.c,v 1.6 1995/06/07 06:45:58 bright Exp $";
#endif

#include "mpid.h"

/* 
 * ADI message recv routines
 *
 *
 * Interface Description
 * ---------------------
 *
 * This file contains the routines that recv messages.  Currently,
 * much of the chameleon code still remains.
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *   void T3D_Set_send_flag( int f )
 *      Sets the debug flag for runtime debugging.
 *
 *   void T3D_Set_recv_debug_flag( int f )
 *      Sets the debug flag for debugging received messages.
 *
 *   void T3D_Set_msg_debug_flag( int f )
 *      Sets debug flag for messages.
 *
 *   int T3D_Get_msg_debug_flag()
 *      Retrieve msg debug flag value.
 *
 *   void T3D_Print_msg_debug()
 *      Print a debugging message.
 *
 *   void T3D_Init_recv_code()
 *      Initialize the recv code.
 *
 *   int T3D_Post_recv(MPIR_RHANDLE *dmpi_recv_handle)
 *      Post a receive.
 *
 *   int T3D_Complete_recv( MPIR_RHANDLE *dmpi_recv_handle ) 
 *      Complete a previously posted receive.
 *
 *   int T3D_Blocking_recv( MPIR_RHANDLE *dmpi_recv_handle ) 
 *      Post and complete a receive.
 *
 *   int T3D_Test_recv( MPIR_RHANDLE *dmpi_recv_handle );
 *      Test for clompletion of a receive.
 */

#include "t3dsend.h"

/****************************************************************************
  Global variables
 ***************************************************************************/
T3D_PKT_T   *t3d_recv_bufs;

/***************************************************************************
   T3D_Set_debug_flag
 ***************************************************************************/
static int DebugFlag = 1;
void T3D_Set_debug_flag( f )
int f;
{
    DebugFlag = f;
}

/***************************************************************************
   T3D_Set_recv_debug_flag
 ***************************************************************************/
void T3D_Set_recv_debug_flag( f )
int f;
{
    DebugFlag = f;
    T3D_Set_sync_debug_flag( f );
}


/***************************************************************************
   T3D_Set_msg_debug_flag
 ***************************************************************************/
static int DebugMsgFlag = 0;
void T3D_Set_msg_debug_flag( f )
int f;
{
    DebugMsgFlag = f;
}

/***************************************************************************
   T3D_Get_msg_debug_flag
 ***************************************************************************/
int T3D_Get_msg_debug_flag()
{
    return DebugMsgFlag;
}


/***************************************************************************
   T3D_Print_msg_debug
 ***************************************************************************/
void T3D_Print_msg_debug()
{
}


/****************************************************************************
  T3D_Recv_packet
 ***************************************************************************/
int T3D_Recv_packet ( pkt )
T3D_PKT_T **pkt;
{
    int         i;
    static int  start;
    int         end;
    int         twice;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Recv_packet\n");
    }
#   endif


    /* clear pkt */
    (*pkt) = (T3D_PKT_T *)0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Recv_packet-looking for a packet\n");
    }
#   endif

    for ( i=start; i<t3d_num_pes; i++ ) {
      if ( i == t3d_myid ) continue;
      if ( ((*pkt) = &t3d_recv_bufs[i])->head.status ) {
#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
            T3D_Printf("T3D_Recv_packet-got message\n");
            T3D_Printf("message contents\n");
	    T3D_Printf("  from       = %d\n",i);
	    T3D_Printf("  status     = %d\n",t3d_recv_bufs[i].head.status);
	    T3D_Printf("  mode       = %d\n",t3d_recv_bufs[i].head.mode);
	    T3D_Printf("  context id = %d\n",t3d_recv_bufs[i].head.context_id);
	    T3D_Printf("  lrank      = %d\n",t3d_recv_bufs[i].head.lrank);
	    T3D_Printf("  tag        = %d\n",t3d_recv_bufs[i].head.tag);
	    T3D_Printf("  length     = %d\n",t3d_recv_bufs[i].head.len);
             }
#         endif
	start = i + 1;
	return i;
      }
    }
    
    if ( start ) 
      for ( i=0; i<start; i++ ) {
      if ( i == t3d_myid ) continue;
	if ( ((*pkt) = &t3d_recv_bufs[i])->head.status ) {
#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
            T3D_Printf("T3D_Recv_packet-got message\n");
            T3D_Printf("message contents\n");
	    T3D_Printf("  from       = %d\n",i);
	    T3D_Printf("  status     = %d\n",t3d_recv_bufs[i].head.status);
	    T3D_Printf("  mode       = %d\n",t3d_recv_bufs[i].head.mode);
	    T3D_Printf("  context id = %d\n",t3d_recv_bufs[i].head.context_id);
	    T3D_Printf("  lrank      = %d\n",t3d_recv_bufs[i].head.lrank);
	    T3D_Printf("  tag        = %d\n",t3d_recv_bufs[i].head.tag);
	    T3D_Printf("  length     = %d\n",t3d_recv_bufs[i].head.len);
             }
#         endif
	  start = i + 1;
	  return i;
	}
    }

    return (-1);

} /* T3D_Recv_packet */


/****************************************************************************
  T3D_Reuse_buf

  Description
    This returns a used buffer to the device.
 ***************************************************************************/
void T3D_Reuse_buf(buf_num)
int buf_num;
{

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Reuse_buf-clearing buffer #%d\n",buf_num);
    }
#   endif

  t3d_recv_bufs[buf_num].head.status = T3D_BUF_AVAIL;

} /* T3D_Reuse_buf */


/***************************************************************************
   T3D_Init_recv_code

   Description:
     This routine is called by the initialization code to preform any 
     receiver initializations, such as preallocating or pre-posting a 
     control-message buffer
 ***************************************************************************/
void T3D_Init_recv_code()
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Init_recv_code\n");
    }
#   endif

}


/****************************************************************************
  T3D_Process_unexpected

  Description 
    This code is called when a receive finds that the message has
    already arrived and has been placed in the unexpected queue.  This
    code stores the information about the message (source, tag,
    length) and copies the message into the receiver's buffer.
 ***************************************************************************/
int T3D_Process_unexpected( dmpi_recv_handle, dmpi_unexpected )
MPIR_RHANDLE *dmpi_recv_handle, *dmpi_unexpected;
{
    MPID_RHANDLE *mpid_recv_handle;
    MPID_RHANDLE *mpid_recv_handle_unex;
    int err = MPI_SUCCESS;
    int completed = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unexpected\n");
    }
#   endif

    /* Can't copy a message until we know it has been completed */

    if ( ! dmpi_unexpected->completer ) {

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unexpected-moving from unexpected queue\n");
    }
#   endif

 
    /* Copy relevant data to recv_handle */
    mpid_recv_handle           = &dmpi_recv_handle->dev_rhandle;
    mpid_recv_handle_unex      = &dmpi_unexpected->dev_rhandle;
    dmpi_recv_handle->source   = dmpi_unexpected->source;
    dmpi_recv_handle->tag      = dmpi_unexpected->tag;
    dmpi_recv_handle->totallen = dmpi_unexpected->totallen;

    /* Copy the message (if there is any data) */
    if (mpid_recv_handle_unex->bytes_as_contig > 0) {
        memcpy( mpid_recv_handle->start, mpid_recv_handle_unex->temp,
                mpid_recv_handle_unex->bytes_as_contig );
    }

    /* if synchronous, then update remote send completed flag */
    if ( (mpid_recv_handle_unex->mode == T3D_PKT_SHORT_SYNC) ||
	 (mpid_recv_handle_unex->mode == T3D_PKT_LONG_SYNC)    )  {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unexpected-setting send completed flag at 0x%x\n",
                    mpid_recv_handle_unex->remote_completed );
    }
#   endif
      shmem_put( (long *)mpid_recv_handle_unex->remote_completed,
		 (long *)&completed,
		 1,
		 mpid_recv_handle_unex->from );
    }
    /* Free temporary space used to store unexpected message */
    if (mpid_recv_handle_unex->temp) {
        T3D_FREE( mpid_recv_handle_unex->temp );
        mpid_recv_handle_unex->temp = 0;
    }
    
    /* Let the soft layer know the send has completed */
    DMPI_mark_recv_completed(dmpi_recv_handle);

    /* Recover dmpi_unexpected. */
    DMPI_free_unexpected( dmpi_unexpected );

    }
    else {

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unexpected-can't move from unexpected queue\n");
    }
#   endif

      err = -1;
    }

    return (err);

} /* T3D_Process_unexpected */


/****************************************************************************
  T3D_Copy_body_short

  Description
    Copy short message from device space to user space.
 ***************************************************************************/
int T3D_Copy_body_short(dmpi_recv_handle, pkt)
MPIR_RHANDLE *dmpi_recv_handle;
T3D_PKT_T    *pkt;
{
    int err    = MPI_SUCCESS;
    int msglen = pkt->head.len;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_short\n");
    }
#   endif

    if ( dmpi_recv_handle->dev_rhandle.bytes_as_contig < msglen ) {
      err = MPI_ERR_TRUNCATE;
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if ( DebugFlag) {
        T3D_Printf("Received a truncated message.\n");
      }
#   endif
      msglen = dmpi_recv_handle->dev_rhandle.bytes_as_contig;
    }

    dmpi_recv_handle->totallen = msglen;

    /* Copy message if needed and mark the receive as completed */
    if ( msglen > 0) 
        memcpy( dmpi_recv_handle->dev_rhandle.start, pkt->short_pkt.buffer,
                msglen ); 

    return (err);

} /* T3D_Copy_body_short */


/****************************************************************************
  T3D_Copy_body_long

  Description
    Copy long message packet from device space to user space.
 ***************************************************************************/
int T3D_Copy_body_long(dmpi_recv_handle, pkt)
MPIR_RHANDLE *dmpi_recv_handle;
T3D_PKT_T    *pkt;
{
    int           err = MPI_SUCCESS;
    MPID_RHANDLE *mpid_recv_handle;
    char         *ptrs[2];

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_long\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;

    mpid_recv_handle->bytes_as_contig = pkt->head.len;
    mpid_recv_handle->mode            = pkt->head.mode; 

    dmpi_recv_handle->totallen        = pkt->head.len;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_long-sending back long buffer ptr\n");
    }
#   endif


    /* 
      The dmpi_recv_handle should all be ready if message expected, and
      buffer of appropriate length will have been allocated and be pointed
      to by mpid_recv_handle->start
    */

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("recv buffer location  = 0x%x\n",pkt->long_pkt.buffer);
	T3D_Printf("recv buffer ptr value = 0x%x\n",mpid_recv_handle->start);
	
        T3D_Printf("completed location  = 0x%x\n",(pkt->long_pkt.buffer + 1 ));
	T3D_Printf("completed value     = 0x%x\n",&(dmpi_recv_handle->completer));   
    }
#   endif

    ptrs[0] = (char *)mpid_recv_handle->start;
    ptrs[1] = (char *)&(dmpi_recv_handle->completer);

    /* 
       Send back to the sending pe (at the location specified in pkt->long.buffer)
       the location of the buffer on this pe to write into and the location of the
       completed flag on this pe.
    */
    shmem_put( (long *)pkt->long_pkt.buffer, 
               (long *)ptrs,
                2, 
                mpid_recv_handle->from );

    return (err);

} /* T3D_Copy_body_long */

/****************************************************************************
  T3D_Copy_body_short_sync

  Description
    Copy short message from device space to user space for synchrounous send.
 ***************************************************************************/
int T3D_Copy_body_short_sync(dmpi_recv_handle, pkt)
MPIR_RHANDLE *dmpi_recv_handle;
T3D_PKT_T    *pkt;
{
    int err       = MPI_SUCCESS;
    int msglen    = pkt->head.len;
    int completed = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_short_sync\n");
    }
#   endif

    if ( dmpi_recv_handle->dev_rhandle.bytes_as_contig < msglen ) {
      err = MPI_ERR_TRUNCATE;
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if ( DebugFlag) {
        T3D_Printf("Received a truncated message.\n");
      }
#   endif
      msglen = dmpi_recv_handle->dev_rhandle.bytes_as_contig;
    }

    dmpi_recv_handle->totallen = msglen;

    /* Copy message if needed and mark the receive as completed */
    if ( msglen > 0) 
        memcpy( dmpi_recv_handle->dev_rhandle.start, pkt->short_pkt.buffer,
                msglen ); 

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if ( DebugFlag) {
        T3D_Printf("Sending completed flag to sender at 0x%x\n",
                   pkt->short_sync_pkt.local_send_completed );
      }
#   endif

    /* Inform sender that receive is completed */
    shmem_put( (long *)pkt->short_sync_pkt.local_send_completed,
	       (long *)&completed,
	       1,
	       dmpi_recv_handle->dev_rhandle.from );

    return (err);

} /* T3D_Copy_body_short_sync */

/****************************************************************************
  T3D_Copy_body_long_sync

  Description
    Copy long message packet from device space to user space for a
    synchronous send
 ***************************************************************************/
int T3D_Copy_body_long_sync(dmpi_recv_handle, pkt)
MPIR_RHANDLE *dmpi_recv_handle;
T3D_PKT_T    *pkt;
{
    int           err = MPI_SUCCESS;
    MPID_RHANDLE *mpid_recv_handle;
    char         *ptrs[2];
    int           completed = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_long_sync\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;

    mpid_recv_handle->bytes_as_contig  = pkt->head.len;
    mpid_recv_handle->mode             = pkt->head.mode; 
    mpid_recv_handle->remote_completed = pkt->long_sync_pkt.local_send_completed;

    dmpi_recv_handle->totallen        = pkt->head.len;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_long_sync-sending back long buffer ptr\n");
    }
#   endif


    /* 
      The dmpi_recv_handle should all be ready if message expected, and
      buffer of appropriate length will have been allocated and be pointed
      to by mpid_recv_handle->start
    */

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("recv buffer location  = 0x%x\n",pkt->long_sync_pkt.buffer);
	T3D_Printf("recv buffer ptr value = 0x%x\n",mpid_recv_handle->start);
	
        T3D_Printf("completed location  = 0x%x\n",(pkt->long_sync_pkt.buffer + 1 ));
	T3D_Printf("completed value     = 0x%x\n",&(dmpi_recv_handle->completer));   
    }
#   endif

    ptrs[0] = (char *)mpid_recv_handle->start;
    ptrs[1] = (char *)&(dmpi_recv_handle->completer);

    /* 
       Send back to the sending pe (at the location specified in pkt->long.buffer)
       the location of the buffer on this pe to write into and the location of the
       completed flag on this pe.
    */
    shmem_put( (long *)pkt->long_sync_pkt.buffer, 
               (long *)ptrs,
                2, 
                mpid_recv_handle->from );

    /* message hasn't been received yet, so can't send send completed flag back
       to source */

    /* Better do this before we look for more messages */
    T3D_Reuse_buf( mpid_recv_handle->from );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_long_sync-waiting for completion of send\n");
    }
#   endif

    while( dmpi_recv_handle->completer )
      T3D_Check_incoming( MPID_NOTBLOCKING );

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if ( DebugFlag) {
        T3D_Printf("T3D_Copy_body_long_sync-Sending completed flag to sender at 0x%x\n",
                   mpid_recv_handle->remote_completed );
      }
#   endif

      /* Inform sender that receive is completed */
    shmem_put( (long *)dmpi_recv_handle->dev_rhandle.remote_completed,
	      (long *)&completed,
	      1,
	      mpid_recv_handle->from );


    DMPI_mark_recv_completed( dmpi_recv_handle );

    return (err);

} /* T3D_Copy_body_long_sync */

/****************************************************************************
  T3D_Copy_body

  Description
    Code to handle copying the body of a packet to user space.
 ***************************************************************************/
int T3D_Copy_body(dmpi_recv_handle, from, pkt)
MPIR_RHANDLE *dmpi_recv_handle;
int           from;
T3D_PKT_T    *pkt;
{
    int            err        = MPI_SUCCESS;
    T3D_Buf_Status buf_status = T3D_BUF_AVAIL;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body\n");
    }
#   endif

    dmpi_recv_handle->dev_rhandle.from = from;

    switch( pkt->head.mode ) {
    case T3D_PKT_SHORT:
      err = T3D_Copy_body_short( dmpi_recv_handle, pkt );
      DMPI_mark_recv_completed( dmpi_recv_handle );
      break;
    case T3D_PKT_LONG:
      err = T3D_Copy_body_long( dmpi_recv_handle, pkt );
      break;
    case T3D_PKT_SHORT_SYNC:
      err = T3D_Copy_body_short_sync( dmpi_recv_handle, pkt );
      DMPI_mark_recv_completed( dmpi_recv_handle );
      break;
    case T3D_PKT_LONG_SYNC:
      err = T3D_Copy_body_long_sync( dmpi_recv_handle, pkt );
      break;
    default:
#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
        if (DebugFlag) {
            T3D_Printf("Unknown Packet Type in T3D_Copy_body\n");
        }
#       endif
      err = MPI_ERR_UNKNOWN;
      break;
    }

    /* Give the buffer back to the device to be used again */
    T3D_Reuse_buf(from);


#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body-freeing remote send buffer #%d at 0x%x\n",
                    T3D_MY_PE,&t3d_dest_bufs[T3D_MY_PE]);
    }
#   endif


    /* Tell sending pe that sending buffer is ready for another send */
    shmem_put( (long *)&t3d_dest_bufs[T3D_MY_PE], 
               (long *)&buf_status, 
                1, 
                from );

    return (err);

} /* T3D_Copy_body */


/****************************************************************************
  T3D_Copy_body_unex_short

  Description
    Copies short message into user space even thought a recv has not been 
    posted.
 ***************************************************************************/
int T3D_Copy_body_unex_short( dmpi_recv_handle, pkt )
MPIR_RHANDLE  *dmpi_recv_handle;
T3D_PKT_T     *pkt;
{
    MPID_RHANDLE *mpid_recv_handle;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_short\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    mpid_recv_handle->bytes_as_contig = pkt->head.len;
    mpid_recv_handle->mode            = pkt->head.mode; 

    dmpi_recv_handle->totallen        = pkt->head.len;

    if ( pkt->head.len > 0 ) {
      mpid_recv_handle->temp          = (char *)T3D_MALLOC(pkt->head.len);

      if ( ! mpid_recv_handle->temp ) {
        T3D_Error("Memory allocation error for unexpected long message\n");
      }

      /* Copy packet contents into temporary buffer */
      memcpy( mpid_recv_handle->temp, pkt->short_pkt.buffer, pkt->head.len );
    }

    return (MPI_SUCCESS);

} /* T3D_Copy_body_unex_short */


/****************************************************************************
  T3D_Copy_body_unex_long

  Description
    Copies long message into user space even thought a recv has not been 
    posted.
 ***************************************************************************/
int T3D_Copy_body_unex_long( dmpi_recv_handle, pkt )
MPIR_RHANDLE  *dmpi_recv_handle;
T3D_PKT_T     *pkt;
{
    int           err = MPI_SUCCESS;
    int          *completed;
    MPID_RHANDLE *mpid_recv_handle;
    char         *send_buffer_ptr;
    char         *ptrs[2];

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;

    mpid_recv_handle->bytes_as_contig = pkt->head.len;
    mpid_recv_handle->mode            = pkt->head.mode; 
   
    dmpi_recv_handle->totallen        = pkt->head.len;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long allocating %d bytes\n",pkt->head.len);
    }
#   endif

    mpid_recv_handle->temp          = (char *)T3D_MALLOC(pkt->head.len);

    if ( ! mpid_recv_handle->temp ) {
      T3D_Error("Memory allocation error for unexpected long message\n");
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long-&dmpi_recv_handle = 0x%x\n",&dmpi_recv_handle);
        T3D_Printf("T3D_Copy_body_unex_long-sending back long buffer ptr\n");
    }
#   endif

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("recv buffer location  = 0x%x\n",pkt->long_pkt.buffer);
        T3D_Printf("recv buffer ptr value = 0x%x\n",mpid_recv_handle->temp);

        T3D_Printf("completed location  = 0x%x\n",pkt->long_pkt.buffer + 1 );
        T3D_Printf("completed value     = 0x%x\n",&(dmpi_recv_handle->completer));
    }
#   endif

    /* 
       Send back to the sending pe (at the location specified in pkt->long.buffer)
       the location of the buffer on this pe to write into and the location of
       the completed flag on this pe.
    */

    ptrs[0] = (char *)mpid_recv_handle->temp;
    ptrs[1] = (char *)&(dmpi_recv_handle->completer);

    shmem_put( (long *)pkt->long_pkt.buffer, 
               (long *)ptrs,
                2, 
                mpid_recv_handle->from );

    return (err);

} /* T3D_Copy_body_unex_long */

/****************************************************************************
  T3D_Copy_body_unex_short_sync

  Description
    Copies short message into user space even thought a recv has not been 
    posted.
 ***************************************************************************/
int T3D_Copy_body_unex_short_sync( dmpi_recv_handle, pkt )
MPIR_RHANDLE  *dmpi_recv_handle;
T3D_PKT_T     *pkt;
{
    MPID_RHANDLE *mpid_recv_handle;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_short_sync\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    mpid_recv_handle->bytes_as_contig = pkt->head.len;
    mpid_recv_handle->mode            = pkt->head.mode; 
    mpid_recv_handle->remote_completed = pkt->short_sync_pkt.local_send_completed;

    dmpi_recv_handle->totallen        = pkt->head.len;

    if ( pkt->head.len > 0 ) {
      mpid_recv_handle->temp          = (char *)T3D_MALLOC(pkt->head.len);

      if ( ! mpid_recv_handle->temp ) {
        T3D_Error("Memory allocation error for unexpected long message\n");
      }

      /* Copy packet contents into temporary buffer */
      memcpy( mpid_recv_handle->temp, pkt->short_sync_pkt.buffer, pkt->head.len );
    }

    return (MPI_SUCCESS);

} /* T3D_Copy_body_unex_short_sync */


/****************************************************************************
  T3D_Copy_body_unex_long_sync

  Description
    Copies long message into user space even thought a recv has not been 
    posted.
 ***************************************************************************/
int T3D_Copy_body_unex_long_sync( dmpi_recv_handle, pkt )
MPIR_RHANDLE  *dmpi_recv_handle;
T3D_PKT_T     *pkt;
{
    int           err = MPI_SUCCESS;
    int          *completed;
    MPID_RHANDLE *mpid_recv_handle;
    char         *send_buffer_ptr;
    char         *ptrs[2];

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long_sync\n");
    }
#   endif


    /* initialize mpid handle */
    mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;

    mpid_recv_handle->bytes_as_contig = pkt->head.len;
    mpid_recv_handle->mode            = pkt->head.mode; 
    mpid_recv_handle->remote_completed = pkt->long_sync_pkt.local_send_completed;
   
    dmpi_recv_handle->totallen        = pkt->head.len;
    dmpi_recv_handle->source          = mpid_recv_handle->from;
    dmpi_recv_handle->tag             = pkt->head.tag;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long_sync allocating %d bytes\n",pkt->head.len);
    }
#   endif

    mpid_recv_handle->temp          = (char *)T3D_MALLOC(pkt->head.len);

    if ( ! mpid_recv_handle->temp ) {
      T3D_Error("Memory allocation error for unexpected long message\n");
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex_long_sync-&dmpi_recv_handle = 0x%x\n",&dmpi_recv_handle);
        T3D_Printf("T3D_Copy_body_unex_long_sync-sending back long buffer ptr\n");
    }
#   endif

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("recv buffer location  = 0x%x\n",pkt->long_sync_pkt.buffer);
        T3D_Printf("recv buffer ptr value = 0x%x\n",mpid_recv_handle->temp);

        T3D_Printf("completed location  = 0x%x\n",pkt->long_sync_pkt.buffer + 1 );
        T3D_Printf("completed value     = 0x%x\n",&(dmpi_recv_handle->completer));
    }
#   endif

    /* 
       Send back to the sending pe (at the location specified in pkt->long.buffer)
       the location of the buffer on this pe to write into and the location of
       the completed flag on this pe.
    */

    ptrs[0] = (char *)mpid_recv_handle->temp;
    ptrs[1] = (char *)&(dmpi_recv_handle->completer);

    shmem_put( (long *)pkt->long_sync_pkt.buffer, 
               (long *)ptrs,
                2, 
                mpid_recv_handle->from );

    return (err);

} /* T3D_Copy_body_unex_long_sync */


/****************************************************************************
  T3D_Copy_body_unex

  Description
    Copies message into user space even thought a recv has not been posted.
 ***************************************************************************/
int T3D_Copy_body_unex( dmpi_recv_handle, from, pkt )
MPIR_RHANDLE  *dmpi_recv_handle;
int            from;
T3D_PKT_T     *pkt;
{
    int            err        = MPI_SUCCESS;
    T3D_Buf_Status buf_status = T3D_BUF_AVAIL;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body_unex\n");
    }
#   endif

    dmpi_recv_handle->dev_rhandle.from = from;

    switch( pkt->head.mode ) {
    case T3D_PKT_SHORT:
      err = T3D_Copy_body_unex_short( dmpi_recv_handle, pkt );
      DMPI_mark_recv_completed(dmpi_recv_handle);
      break;
    case T3D_PKT_LONG:
      err = T3D_Copy_body_unex_long( dmpi_recv_handle, pkt );
      break;
    case T3D_PKT_SHORT_SYNC:
      err = T3D_Copy_body_unex_short_sync( dmpi_recv_handle, pkt );
      DMPI_mark_recv_completed(dmpi_recv_handle);
      break;
    case T3D_PKT_LONG_SYNC:
      err = T3D_Copy_body_unex_long_sync( dmpi_recv_handle, pkt );
      break;
    default:
#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
        if (DebugFlag) {
            T3D_Printf("Unknown Packet Type in T3D_Copy_body_unex\n");
        }
#       endif
      err = MPI_ERR_UNKNOWN;
      break;
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_body-freeing remote send buffer #%d at 0x%x\n",
                    T3D_MY_PE,&t3d_dest_bufs[T3D_MY_PE]);
    }
#   endif

    /* Give the buffer back to the device to be used again */
    T3D_Reuse_buf(from);


    /* Tell sending pe that sending buffer is ready for another send */
    shmem_put( (long *)&t3d_dest_bufs[T3D_MY_PE], 
               (long *)&buf_status, 
                1, 
                from );

    return (err);
} /* T3D_Copy_body_unex */


/****************************************************************************
  T3D_Check_incoming

  Description
    This checks for incoming messages and dispatches them.  
    It is similar to T3D_Blocking_recv which is optimized for 
    the important case of blocking receives for a particular message.


        blocking - true if this routine should block until a message is
                   available

    Returns -1 if nonblocking and no messages pending
 ***************************************************************************/
int T3D_Check_incoming( blocking )
int blocking;
{
    T3D_PKT_T       *pkt;
    MPIR_RHANDLE    *dmpi_recv_handle;
    int              is_posted;
    int              err = MPI_SUCCESS;
    int              buf_num = -1;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Check_incoming\n");
    }
#   endif


    /* If nonblocking and no headers available, exit */
    if (blocking == MPID_NOTBLOCKING) {
        if ((buf_num = T3D_Recv_packet((T3D_PKT_T **)&pkt)) < 0) return (-1);
    }
    else {
        while ((buf_num = T3D_Recv_packet((T3D_PKT_T **)&pkt)) < 0);
    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Check_incoming-message received\n");
    }
#   endif


    /* Is the message expected or not? 
       This routine RETURNS a dmpi_recv_handle, creating one if the message 
       is unexpected (is_posted == 0) */
    DMPI_msg_arrived( pkt->head.lrank, pkt->head.tag, 
                      pkt->head.context_id, 
                      &dmpi_recv_handle, &is_posted );
    if (is_posted) {
        err = T3D_Copy_body( dmpi_recv_handle, buf_num, (T3D_PKT_T *)pkt );
    }
    else {
        err = T3D_Copy_body_unex( dmpi_recv_handle, buf_num, (T3D_PKT_T *)pkt );
    }

    return (err);

} /* T3D_Check_incoming */


/***************************************************************************
   T3D_Post_recv

 ***************************************************************************/
int T3D_Post_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
    MPIR_RHANDLE *dmpi_unexpected;
    int          found, err;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_recv\n");
    }
#   endif

    /* Check to see if the message has already been received. */
    DMPI_search_unexpected_queue( dmpi_recv_handle->source,
                                  dmpi_recv_handle->tag,
                                  dmpi_recv_handle->contextid,
                                  &found, 1, &dmpi_unexpected );

    /* If found, move it to the expected queue */
    if (found) {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("T3D_Post_recv-message found but not completed\n");
      }
#   endif
       while ( dmpi_unexpected->completer )
	 T3D_Check_incoming( MPID_NOTBLOCKING );
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
        T3D_Printf("T3D_Post_recv-message found is now complete\n");
      }
#   endif
      return(T3D_Process_unexpected(dmpi_recv_handle, dmpi_unexpected));
    }
    /* If it's not found, enqueue the posted receive */ 
    else {
        /* Add to the posted receive queue */
        MPIR_enqueue( &MPIR_posted_recvs, dmpi_recv_handle, MPIR_QRHANDLE );
    }

    /* Drain incoming packets */
    T3D_Check_incoming( MPID_NOTBLOCKING );

    /* Note that at this point, the message MAY be here by is_available is
       still zero.  This is ok, since is_available is intended as an
       optimization */


    /* Return is_available instead??? */
    return MPI_SUCCESS;

}  /* T3D_Post_recv */


/***************************************************************************
   T3D_Complete_recv

   Description
     This routine completes a particular receive.  It does this by processing
     incoming messages until the indicated message is received.

     For fairness, we may want a version with an array of handles.
 ***************************************************************************/
int T3D_Complete_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
    MPIR_RHANDLE    *dmpi_unexpected;
    int              found, tag, source, context_id;
    int              err = MPI_SUCCESS;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Complete_recv\n");
    }
#   endif
/*
    if ( dmpi_recv_handle->completer ) {

      tag        = dmpi_recv_handle->tag;
      context_id = dmpi_recv_handle->contextid;
      source     = dmpi_recv_handle->source;
    
      DMPI_search_unexpected_queue( source, tag, context_id, 
				   &found, 1, &dmpi_unexpected );
      if (found) {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	if (DebugFlag) {
	  T3D_Printf("T3D_Complete_recv-message found but not complete\n");
	}
#   endif
        while ( dmpi_unexpected->completer)
	  T3D_Check_incoming( MPID_NOTBLOCKING );
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	if (DebugFlag) {
	  T3D_Printf("T3D_Complete_recv-message found is now complete\n");
	}
#   endif
	return T3D_Process_unexpected(dmpi_recv_handle,dmpi_unexpected);
      }
    }
    
    T3D_Blocking_recv( dmpi_recv_handle );
*/

    while( dmpi_recv_handle->completer )
       T3D_Check_incoming( MPID_NOTBLOCKING );

    return MPI_SUCCESS;
}


/***************************************************************************
   T3D_Blocking_recv
 ***************************************************************************/
int T3D_Blocking_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
    T3D_PKT_T       *pkt;
    MPIR_RHANDLE    *dmpi_unexpected, *dmpi_save_recv_handle;
    int              found, is_posted, tag, source, context_id;
    int              tagmask, srcmask;
    int              ptag, pcid, plrk;   /* Values from packet */
    int              err = MPI_SUCCESS;
    int              buf_num;
    int              completed = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Blocking_recv\n");
    }
#   endif

    
    /* At this time, check to see if the message has already been received */
    tag        = dmpi_recv_handle->tag;
    context_id = dmpi_recv_handle->contextid;
    source     = dmpi_recv_handle->source;
    
    DMPI_search_unexpected_queue( source, tag, context_id, 
                                  &found, 1, &dmpi_unexpected );
    if (found) {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
	T3D_Printf("T3D_Blocking_recv-message found but not complete\n");
      }
#   endif
        while ( dmpi_unexpected->completer)
	  T3D_Check_incoming( MPID_NOTBLOCKING );
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
	T3D_Printf("T3D_Blocking_recv-message found is now complete\n");
      }
#   endif
	return T3D_Process_unexpected(dmpi_recv_handle,dmpi_unexpected);
    }

    /* If we got here, the message is not yet available */
    dmpi_save_recv_handle = dmpi_recv_handle;
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

    /* While the message has not been received ... */
    while (dmpi_save_recv_handle->completer) {

        /* get a packet */
        if ((buf_num = T3D_Recv_packet(&pkt)) >= 0) {

          /* process it */
          ptag = pkt->head.tag;
          plrk = pkt->head.lrank;
          pcid = pkt->head.context_id;

          /* We should check the size here for internal errors .... */

          /* If the packet contains the message we're looking for */
          if (pcid            == context_id && 
             (ptag & tagmask) == tag        &&
             (plrk & srcmask) == source        ) {
            /* Found the message I'm waiting for(it wasn't queued) */
            is_posted                = 1;
            dmpi_recv_handle         = dmpi_save_recv_handle;
            dmpi_recv_handle->tag    = ptag;
            dmpi_recv_handle->source = plrk;
          }
          /* If the packet doesn't contain the message we're looking for */
          else 

            DMPI_msg_arrived( plrk, ptag, pcid,
                              &dmpi_recv_handle, &is_posted );

          /* Copy the body of the message into user space */
          if (is_posted) {
            err = T3D_Copy_body( dmpi_recv_handle, buf_num, pkt);
          }
          else {
            err = T3D_Copy_body_unex( dmpi_recv_handle, buf_num, pkt );
          }
        }
    }
    
    if ( (dmpi_recv_handle->dev_rhandle.mode == T3D_PKT_SHORT_SYNC ) ||
	 (dmpi_recv_handle->dev_rhandle.mode == T3D_PKT_LONG_SYNC  )    )  {
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Blocking_recv-sending completed flag back to sender at 0x%x\n",
                   dmpi_recv_handle->dev_rhandle.remote_completed );
    }
#   endif
  }

    
    return (err);

} /* T3D_Blocking_recv */


/***************************************************************************
   T3D_Test_recv

   Description:
      This routine tests for a receive to be completed.
 ***************************************************************************/
int T3D_Test_recv( dmpi_recv_handle )
MPIR_RHANDLE *dmpi_recv_handle;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Test_recv\n");
    }
#   endif

    return ( T3D_Complete_recv( dmpi_recv_handle ) );
}



