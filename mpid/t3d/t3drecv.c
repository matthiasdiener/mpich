/*
 *  $Id: t3drecv.c,v 1.10 1995/09/15 19:19:19 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */
 
#ifndef lint
static char vcid[] = "$Id: t3drecv.c,v 1.10 1995/09/15 19:19:19 bright Exp $";
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

/****************************************************************************
  Global variables
 ***************************************************************************/
#pragma _CRI cache_align t3d_recv_bufs
volatile T3D_PKT_T   *t3d_recv_bufs;
extern MPIR_QHDR      T3D_long_sends;

/***************************************************************************
   T3D_Set_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;
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

/* FORTRAN equivalent */
void T3D_SET_RECV_DEBUG_FLAG()
{
    DebugFlag = 1;
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
volatile T3D_PKT_T **pkt;
{
    static int  last=0;
    int         i,start,buf_num;

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

    buf_num = start = ( i = last + 1 ) % t3d_num_pes;
    do {
      if ( buf_num != T3D_MY_PE )
	if ( t3d_recv_bufs[buf_num].head.status != T3D_BUF_AVAIL ) {
	  *pkt = &t3d_recv_bufs[buf_num];
#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("T3D_Recv_packet-got message\n");
	    T3D_Printf("message contents\n");
	    T3D_Printf("  from       = %d\n",buf_num);
	    T3D_Printf("  status     = %d\n",t3d_recv_bufs[buf_num].head.status);
	    T3D_Printf("  mode       = %d\n",t3d_recv_bufs[buf_num].head.mode);
	    T3D_Printf("  context id = %d\n",t3d_recv_bufs[buf_num].head.context_id);
	    T3D_Printf("  lrank      = %d\n",t3d_recv_bufs[buf_num].head.lrank);
	    T3D_Printf("  tag        = %d\n",t3d_recv_bufs[buf_num].head.tag);
	    T3D_Printf("  length     = %d\n",t3d_recv_bufs[buf_num].head.len);
	  }
#       endif
	  return ( last = buf_num );
	}
      last = buf_num = ++i % t3d_num_pes;
    } while ( buf_num != start );

    return (-1);

} /* T3D_Recv_packet */

/****************************************************************************
  T3D_Copy_unexpected

  Description 
    This code is called when a receive finds that the message has
    already arrived and has been placed in the unexpected queue.  This
    code stores the information about the message (source, tag,
    length) and copies the message into the receiver's buffer.
 ***************************************************************************/
int T3D_Copy_unexpected( dmpi_recv_handle )
MPIR_RHANDLE *dmpi_recv_handle;
{
    MPID_RHANDLE *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    MPIR_RHANDLE *dmpi_unexpected  = (MPIR_RHANDLE *)mpid_recv_handle->dmpi_unexpected;
    MPID_RHANDLE *mpid_unexpected  = &dmpi_unexpected->dev_rhandle;
    int           err = MPI_SUCCESS;
    int           completed = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Copy_unexpected\n");
    }
#   endif

    /* Copy relevant data to recv_handle */
    dmpi_recv_handle->source   = dmpi_unexpected->source;
    dmpi_recv_handle->tag      = dmpi_unexpected->tag;
    if (mpid_unexpected->bytes_as_contig <= mpid_recv_handle->bytes_as_contig)
        dmpi_recv_handle->totallen = mpid_unexpected->bytes_as_contig;
    else
    {
        dmpi_recv_handle->totallen = mpid_recv_handle->bytes_as_contig;
        err = dmpi_recv_handle->errval = MPI_ERR_TRUNCATE;
    }

    mpid_recv_handle->bytes_as_contig = dmpi_recv_handle->totallen;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
	T3D_Printf(" unexpected =\n");
	T3D_Printf("   temp        = 0x%x\n",mpid_unexpected->temp);
	T3D_Printf("   mode        = %d\n",  mpid_unexpected->mode);
	T3D_Printf("   from        = %d\n",  mpid_unexpected->from);
	T3D_Printf("   bytes       = %d\n",  mpid_unexpected->bytes_as_contig);
	T3D_Printf(" posted =\n");
	T3D_Printf("   mode        = %d\n",  mpid_recv_handle->mode);
	T3D_Printf("   from        = %d\n",  mpid_recv_handle->from);
	T3D_Printf("   start       = 0x%x\n",mpid_recv_handle->start);
	T3D_Printf("   bytes       = %d\n",  mpid_recv_handle->bytes_as_contig);
    }
#   endif

    if ( (mpid_unexpected->mode == T3D_PKT_SHORT_SYNC) ||
	 (mpid_unexpected->mode == T3D_PKT_LONG_SYNC )    ) {
	
#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("  Sending send completed flag to 0x%x\n",
		   mpid_unexpected->sync_recv_completed );
      }
#     endif

      T3D_CHECK_TARGET( mpid_unexpected->sync_recv_completed, 8 ); 

      shmem_put( (long *)mpid_unexpected->sync_recv_completed,
		 (long *)&completed,
		 1,
		 mpid_unexpected->from );

      T3D_RESET_STACK;

    }

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
      T3D_Printf("  Copying from temp buffer\n");
      T3D_Printf("     start = 0x%x\n",mpid_recv_handle->start);
      T3D_Printf("     temp  = 0x%x\n",mpid_unexpected->temp);
    }
#   endif

    if ( mpid_recv_handle->bytes_as_contig > 0 ) { 
      T3D_MEMCPY( mpid_recv_handle->start,
		  mpid_unexpected->temp,
		  mpid_recv_handle->bytes_as_contig );
    }

    /* Free temporary space used to store unexpected message */
    if (mpid_unexpected->temp != (void *)NULL) {
      T3D_FREE( mpid_unexpected->temp );
      mpid_unexpected->temp = 0;
    }
    
    /* Let the soft layer know the recv has completed */
    DMPI_mark_recv_completed(dmpi_recv_handle);
    
    /* Recover unexpected handle. */
    DMPI_free_unexpected( dmpi_unexpected );
    
    return (err);

} /* T3D_Copy_unexpected */

/****************************************************************************
  T3D_Process_unex_packet

  Description
    Process an unexpected message
 ***************************************************************************/
int T3D_Process_unex_packet(dmpi_recv_handle, from, pkt )
MPIR_RHANDLE *dmpi_recv_handle;
int           from;
T3D_PKT_T    *pkt;
{
    T3D_Long_Send_Info  t3d_long_send_info;
    int                 err              = MPI_SUCCESS;
    T3D_Buf_Status      buf_status       = T3D_BUF_AVAIL;
    MPID_RHANDLE       *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    long               *long_send_info_target; 
    int                 msglen;
    int                 buf_loc;


#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unex_packet\n");
    }
#   endif

    /* Copy relevant info from packet to device handle */
    dmpi_recv_handle->source          = pkt->head.lrank;
    dmpi_recv_handle->tag             = pkt->head.tag;
    dmpi_recv_handle->totallen        = pkt->head.len;
    
    mpid_recv_handle->from            = from; 
    mpid_recv_handle->mode            = pkt->head.mode; 
    mpid_recv_handle->bytes_as_contig = dmpi_recv_handle->totallen;

    if ( mpid_recv_handle->bytes_as_contig > 0) {
        if ( (mpid_recv_handle->temp =
	      (char *)T3D_MALLOC( mpid_recv_handle->bytes_as_contig ) ) == (char *)NULL ) {
	    T3D_Printf("Memory allocation error\n");
	    T3D_Abort( T3D_ERR_MEMORY );
	}
    }

    switch( pkt->head.mode ) {
    
    case T3D_PKT_SHORT_SYNC:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("   Processing short synchronous packet\n");
      }
#     endif

      /* save location of recv completed flag */
      while ( pkt->short_sync_pkt.local_send_completed == (char *)NULL );
      mpid_recv_handle->sync_recv_completed = pkt->short_sync_pkt.local_send_completed;

      if ( mpid_recv_handle->bytes_as_contig > 0 ) {
	buf_loc = T3D_BUFFER_LENGTH - T3D_MSG_LEN_32(mpid_recv_handle->bytes_as_contig) * 4;
	T3D_MEMCPY( mpid_recv_handle->temp,
		    &(pkt->short_sync_pkt.buffer[buf_loc]),
		    mpid_recv_handle->bytes_as_contig );
      }
        
      DMPI_mark_recv_completed( dmpi_recv_handle );

      break;
      
    case T3D_PKT_SHORT:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("   Processing short packet\n");
      }
#     endif

      if ( mpid_recv_handle->bytes_as_contig > 0 ) {
	buf_loc = T3D_BUFFER_LENGTH - T3D_MSG_LEN_32(mpid_recv_handle->bytes_as_contig) * 4;
	T3D_MEMCPY( mpid_recv_handle->temp,
		    &(pkt->short_pkt.buffer[buf_loc]),
		    mpid_recv_handle->bytes_as_contig );
      }
      
      DMPI_mark_recv_completed( dmpi_recv_handle );

      break;
      
    case T3D_PKT_LONG_SYNC:
    case T3D_PKT_LONG:


      if ( pkt->head.mode == T3D_PKT_LONG_SYNC ) {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("   Processing long synchronous packet\n");
      }
#     endif

	/* save location of recv completed flag */
	while ( pkt->long_sync_pkt.local_send_completed == (char *)NULL );
	mpid_recv_handle->sync_recv_completed = pkt->long_sync_pkt.local_send_completed;
	t3d_long_send_info.completer_value = -1;
	long_send_info_target = (long *)pkt->long_sync_pkt.long_send_info_target;
      }
      else {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("   Processing long packet\n");
      }
#     endif

	t3d_long_send_info.completer_value = 0;
	long_send_info_target = (long *)pkt->long_pkt.long_send_info_target;
      }

      t3d_long_send_info.target_buffer    = mpid_recv_handle->temp;
      t3d_long_send_info.target_completer = (char *)&dmpi_recv_handle->completer;
      t3d_long_send_info.length           = mpid_recv_handle->bytes_as_contig;


      T3D_CHECK_TARGET( long_send_info_target,
		        sizeof( T3D_Long_Send_Info ) );

      msglen = T3D_MSG_LEN_64( sizeof(T3D_Long_Send_Info) );
 
      shmem_put( long_send_info_target,
		 (long *)&t3d_long_send_info,
		 msglen,
		 mpid_recv_handle->from );

      T3D_RESET_STACK;
      
#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
        T3D_Printf("Sent long_send_info to 0x%x\n",long_send_info_target);
	T3D_Printf("long_send_info =\n");
	T3D_Printf("  target_buffer     = 0x%x\n",t3d_long_send_info.target_buffer   );
	T3D_Printf("  target_completer  = 0x%x\n",t3d_long_send_info.target_completer);
	T3D_Printf("  completer_value   = %d\n",  t3d_long_send_info.completer_value );
	T3D_Printf("  length            = %d\n",  t3d_long_send_info.length          );
      }
#     endif

      break;
      
    default:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("Unknown Packet Type in T3D_Process_unex_packet\n");
      }
#     endif

      err = MPI_ERR_UNKNOWN;

      break;

    }

    /* Give the buffer back to the device to be used again */
    t3d_recv_bufs[ from ].short_sync_pkt.local_send_completed =
    t3d_recv_bufs[ from ].long_sync_pkt.local_send_completed  = (char *)NULL;
    t3d_recv_bufs[ from ].head.status = T3D_BUF_AVAIL;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_unex_packet-freeing remote send buffer #%d at 0x%x\n",
                    T3D_MY_PE,&t3d_dest_bufs[T3D_MY_PE]);
    }
#   endif

    /* Tell sending pe that sending buffer is ready for another send */
    shmem_put( (long *)&t3d_dest_bufs[T3D_MY_PE], 
               (long *)&buf_status, 
                1, 
                from );

    return (err);

} /* T3D_Process_unex_packet */

/****************************************************************************
  T3D_Process_packet

  Description
    Process an expected message
 ***************************************************************************/
int T3D_Process_packet(dmpi_recv_handle, from, pkt )
MPIR_RHANDLE *dmpi_recv_handle;
int           from;
T3D_PKT_T    *pkt;
{
    T3D_Long_Send_Info  t3d_long_send_info;
    int                 err              = MPI_SUCCESS;
    T3D_Buf_Status      buf_status       = T3D_BUF_AVAIL;
    MPID_RHANDLE       *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    int                 completed        = 0;
    long               *long_send_info_target;
    int                 msglen;
    int                 buf_loc;


#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_packet\n");
    }
#   endif

    /* Copy relevant info from packet to device handle */
    dmpi_recv_handle->source          = pkt->head.lrank;
    dmpi_recv_handle->tag             = pkt->head.tag;

    if (mpid_recv_handle->bytes_as_contig >= pkt->head.len)
        dmpi_recv_handle->totallen = pkt->head.len;
    else
    {
        dmpi_recv_handle->totallen = mpid_recv_handle->bytes_as_contig;
        err = dmpi_recv_handle->errval = MPI_ERR_TRUNCATE;
    }
 
    mpid_recv_handle->from            = from; 
    mpid_recv_handle->mode            = pkt->head.mode; 
    mpid_recv_handle->bytes_as_contig = dmpi_recv_handle->totallen;


    switch( pkt->head.mode ) {
    
    case T3D_PKT_SHORT_SYNC:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("   Processing short synchronous packet\n");
      }
#     endif

      while ( pkt->short_sync_pkt.local_send_completed == (char *)NULL );
      mpid_recv_handle->sync_recv_completed = pkt->short_sync_pkt.local_send_completed;

      
#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("  sending recv completed flag to sender\n");
      }
#     endif

      /* let sender know we are receiving */
      T3D_CHECK_TARGET( mpid_recv_handle->sync_recv_completed, 8 );
	  
      shmem_put( (long *)mpid_recv_handle->sync_recv_completed,
		 (long *)&completed,
		 1,
		 mpid_recv_handle->from );

      T3D_RESET_STACK;

      if ( mpid_recv_handle->bytes_as_contig > 0 ) {
	buf_loc = T3D_BUFFER_LENGTH - T3D_MSG_LEN_32(mpid_recv_handle->bytes_as_contig) * 4;
	T3D_MEMCPY( mpid_recv_handle->start,
		    &(pkt->short_sync_pkt.buffer[buf_loc]),
		    mpid_recv_handle->bytes_as_contig );
      }
      
      DMPI_mark_recv_completed( dmpi_recv_handle );

      break;

    case T3D_PKT_SHORT:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("   Processing short packet\n");
      }
#     endif

      if ( mpid_recv_handle->bytes_as_contig > 0 ) {
	buf_loc = T3D_BUFFER_LENGTH - T3D_MSG_LEN_32(mpid_recv_handle->bytes_as_contig) * 4;
	T3D_MEMCPY( mpid_recv_handle->start,
		    &(pkt->short_pkt.buffer[buf_loc]),
		    mpid_recv_handle->bytes_as_contig );
      }
      
      DMPI_mark_recv_completed( dmpi_recv_handle );

      break;
      
    case T3D_PKT_LONG_SYNC:

    case T3D_PKT_LONG:

      if ( pkt->head.mode == T3D_PKT_LONG_SYNC ) {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("   Processing long synchronous packet\n");
      }
#     endif

	while ( pkt->long_sync_pkt.local_send_completed == (char *)NULL );
	mpid_recv_handle->sync_recv_completed = pkt->long_sync_pkt.local_send_completed;
	t3d_long_send_info.completer_value = -1;
	long_send_info_target = (long *)pkt->long_sync_pkt.long_send_info_target;
      }
      else {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("   Processing long packet\n");
      }
#     endif

	t3d_long_send_info.completer_value = 0;
	long_send_info_target = (long *)pkt->long_pkt.long_send_info_target;
      }

      /* Hopefully we didn't match this up with a posted recv of length 0 */
      if ( mpid_recv_handle->bytes_as_contig > 0 ) {

	if ( IS_4BYTE_ALIGNED(mpid_recv_handle->start) &&
	    ((mpid_recv_handle->bytes_as_contig % 4) == 0) ) {
	  mpid_recv_handle->temp = mpid_recv_handle->start;

#        if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
         if (DebugFlag) {
           T3D_Printf("   buffer is aligned and length is multiple of 4\n");
       	 }
#        endif

	}
	else {

#        if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
         if (DebugFlag) {
           T3D_Printf("   buffer not aligned or length not multiple of 4\n");
	 }
#        endif

	  if ( (mpid_recv_handle->temp = (void *)T3D_MALLOC( mpid_recv_handle->bytes_as_contig))
	      == (void *)NULL ) {
	    T3D_Printf("Memory allocation error\n");
	    T3D_Abort( T3D_ERR_MEMORY );
	  }
	  mpid_recv_handle->needs_copy = 1;
	  t3d_long_send_info.completer_value--;
	}
      }
      else {
	DMPI_mark_recv_completed( dmpi_recv_handle );
      }

      t3d_long_send_info.target_buffer    = mpid_recv_handle->temp;
      t3d_long_send_info.target_completer = (char *)&dmpi_recv_handle->completer;
      t3d_long_send_info.length           = mpid_recv_handle->bytes_as_contig;

      T3D_CHECK_TARGET( long_send_info_target,
		        sizeof( T3D_Long_Send_Info ) );

      msglen = T3D_MSG_LEN_64(sizeof( T3D_Long_Send_Info ));

      shmem_put( long_send_info_target,
		 (long *)&t3d_long_send_info,
		 msglen, 
		 mpid_recv_handle->from );

      T3D_RESET_STACK;
      
#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
	T3D_Printf("Sent long_send_info to 0x%x\n",long_send_info_target);
	T3D_Printf("long_send_info =\n");
	T3D_Printf("  target_buffer     = 0x%x\n",t3d_long_send_info.target_buffer   );
	T3D_Printf("  target_completer  = 0x%x\n",t3d_long_send_info.target_completer);
	T3D_Printf("  completer_value   = %d\n",  t3d_long_send_info.completer_value );
	T3D_Printf("  length            = %d\n",  t3d_long_send_info.length          );
      }
#     endif

      break;

    default:

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
          T3D_Printf("Unknown Packet Type in T3D_Process_packet\n");
      }
#     endif

      err = MPI_ERR_UNKNOWN;

      break;

    }

    /* Give the buffer back to the device to be used again */
    t3d_recv_bufs[ from ].short_sync_pkt.local_send_completed =
    t3d_recv_bufs[ from ].long_sync_pkt.local_send_completed  = (char *)NULL;
    t3d_recv_bufs[ from ].head.status = T3D_BUF_AVAIL;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Process_packet-freeing remote send buffer #%d at 0x%x\n",
                    T3D_MY_PE,&t3d_dest_bufs[T3D_MY_PE]);
    }
#   endif

    /* Tell sending pe that sending buffer is ready for another send */
    shmem_put( (long *)&t3d_dest_bufs[T3D_MY_PE], 
               (long *)&buf_status, 
                1, 
                from );

    return (err);

} /* T3D_Process_packet */


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
    int              err     = MPI_SUCCESS;
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


    /* Add it with global source, because that's what you look for it with */
    DMPI_msg_arrived( buf_num, pkt->head.tag, 
                      pkt->head.context_id, 
                      &dmpi_recv_handle, &is_posted );

    if ( is_posted )
      T3D_Process_packet( dmpi_recv_handle, buf_num, (T3D_PKT_T *)pkt );
    else
      T3D_Process_unex_packet( dmpi_recv_handle, buf_num, (T3D_PKT_T *)pkt );

    return (err);

} /* T3D_Check_incoming */


/***************************************************************************
   T3D_Post_recv

 ***************************************************************************/
int T3D_Post_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{
    MPID_RHANDLE *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    MPIR_RHANDLE *dmpi_unexpected;
    int           found;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Post_recv\n");
    }
#   endif

    /* Check the unexpected queue to see if message has arrived */
    DMPI_search_unexpected_queue( dmpi_recv_handle->source,
		                  dmpi_recv_handle->tag,
				  dmpi_recv_handle->contextid,
				  &found, 1, 
				  &dmpi_unexpected );

    if ( found ) {

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
        T3D_Printf("   Found it in the unexpected queue\n");
      }
#     endif

      mpid_recv_handle->dmpi_unexpected = dmpi_unexpected;

      mpid_recv_handle->mode = T3D_UNEXPECTED_RECV;

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
        T3D_Printf("   Checking for completion of unexpected msg\n");
      }
#     endif

      if (dmpi_unexpected->completer <= 0 ) {

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
        if (DebugFlag) {
          T3D_Printf("   unexpected recv is complete\n");
        }
#       endif

        T3D_Copy_unexpected( dmpi_recv_handle );

        return dmpi_recv_handle->errval;

      }
      else  {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
          if (DebugFlag) {
              T3D_Printf("   unexpected recv is not complete\n");
          }
#         endif

      }
    } 
    else {
    /* if it's not found, look for it */

#     if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
      if (DebugFlag) {
        T3D_Printf("   Didn't find it in the unexpected queue\n");
      }
#     endif

      MPIR_enqueue( &MPIR_posted_recvs, (MPIR_COMMON *)dmpi_recv_handle, 
		    MPIR_QRHANDLE );

      T3D_Check_incoming( MPID_NOTBLOCKING );
    }

    /* Return is_available instead??? */
    return MPI_SUCCESS;

}  /* T3D_Post_recv */


/***************************************************************************
   T3D_Blocking_recv
 ***************************************************************************/
int T3D_Blocking_recv( dmpi_recv_handle ) 
MPIR_RHANDLE *dmpi_recv_handle;
{

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Blocking_recv\n");
    }
#   endif

    T3D_Post_recv( dmpi_recv_handle );
    
    T3D_Complete_recv( dmpi_recv_handle );

    return dmpi_recv_handle->errval;

} /* T3D_Blocking_recv */


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

    MPID_RHANDLE *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    MPIR_RHANDLE *dmpi_unexpected;
    int           found            = 0;
    int           completed        = 0;
    

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("Complete recv\n");
    }
#   endif

    if ( T3D_long_sends.currlen ) T3D_Process_long_sends();

    if ( dmpi_recv_handle->completer != 0 ) {

      /* short message not found when posted, but could be here now */
      if (mpid_recv_handle->mode == -1 ) { /* not been processed */

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	if (DebugFlag) {
	  T3D_Printf("  Posted recv hasn't been processed\n");
	  T3D_Printf("  Checking the unexpected queue\n");
	}
#         endif

	/* Check to see if the message has already been received. */
	DMPI_search_unexpected_queue( dmpi_recv_handle->source,
				     dmpi_recv_handle->tag,
				     dmpi_recv_handle->contextid,
				     &found, 1, 
				     &dmpi_unexpected );

	if ( found ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Found it in the unexpected queue\n");
	  }
#         endif

	  mpid_recv_handle->dmpi_unexpected = dmpi_unexpected;

	  mpid_recv_handle->mode = T3D_UNEXPECTED_RECV;
	}
	else {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Didn't find it in the unexpected queue\n");
	  }
#         endif

	}

      }

      if ( mpid_recv_handle->mode == T3D_UNEXPECTED_RECV ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
            T3D_Printf("   Waiting for completion of unexpected msg\n");
          }
#         endif

	  dmpi_unexpected = mpid_recv_handle->dmpi_unexpected;

          while( (volatile)dmpi_unexpected->completer > 0 )
            T3D_Check_incoming( MPID_NOTBLOCKING );

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
          if (DebugFlag) {
            T3D_Printf("   unexpected recv is complete\n");
          }
#         endif

	  T3D_Copy_unexpected(dmpi_recv_handle);

          return dmpi_recv_handle->errval;

	}

      while( (volatile)dmpi_recv_handle->completer > 0 ) {
	T3D_Check_incoming( MPID_NOTBLOCKING );
	if ( T3D_long_sends.currlen ) T3D_Process_long_sends();
      }

      if ( dmpi_recv_handle->completer < 0 ) {

	if ( mpid_recv_handle->mode == T3D_PKT_SHORT_SYNC ||
	     mpid_recv_handle->mode == T3D_PKT_LONG_SYNC     ) {
	
	  T3D_CHECK_TARGET( mpid_recv_handle->sync_recv_completed, 8 );

	  shmem_put( (long *)mpid_recv_handle->sync_recv_completed,
		     (long *)&completed,
		     1,
		     mpid_recv_handle->from );
      
          T3D_RESET_STACK;

	}

	if ( mpid_recv_handle->needs_copy ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Copying to misaligned buffer\n");
	    T3D_Printf("      start = 0x%x\n",mpid_recv_handle->start);
	    T3D_Printf("      temp  = 0x%x\n",mpid_recv_handle->temp);
	  }
#         endif

	  memcpy( mpid_recv_handle->start,
		  mpid_recv_handle->temp,
		  mpid_recv_handle->bytes_as_contig );

	  T3D_FREE( mpid_recv_handle->temp );

	}

	DMPI_mark_recv_completed( dmpi_recv_handle );

      }
    }

#  if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
   if (DebugFlag) {
     if ( dmpi_recv_handle->completer == 0 )
       T3D_Printf("  Message recv is complete\n");
     else
       T3D_Printf("  Message recv is not complete\n");
   }
#  endif

    return dmpi_recv_handle->errval;
}


/***************************************************************************
   T3D_Test_recv

   Description:
      This routine tests for a receive to be completed.
 ***************************************************************************/
int T3D_Test_recv( dmpi_recv_handle )
MPIR_RHANDLE *dmpi_recv_handle;
{

    MPID_RHANDLE *mpid_recv_handle = &dmpi_recv_handle->dev_rhandle;
    MPIR_RHANDLE *dmpi_unexpected;
    int           found            = 0;
    int           completed        = 0;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
    if (DebugFlag) {
        T3D_Printf("T3D_Test_recv\n");
    }
#   endif


    if ( dmpi_recv_handle->completer != 0 ) {

      if ( T3D_long_sends.currlen ) T3D_Process_long_sends();

      /* short message not found when posted, but could be here now */
      if (mpid_recv_handle->mode == -1 ) { /* not been processed */

#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	if (DebugFlag) {
	  T3D_Printf("  Posted recv hasn't been processed\n");
	  T3D_Printf("  Checking the unexpected queue\n");
	}
#         endif

	/* Probe unexpected queue to see if message has arrived */
	DMPI_search_unexpected_queue( dmpi_recv_handle->source,
				     dmpi_recv_handle->tag,
				     dmpi_recv_handle->contextid,
				     &found, 1, 
				     &dmpi_unexpected );

	if ( found ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Found it in the unexpected queue\n");
	  }
#         endif

	  mpid_recv_handle->dmpi_unexpected = dmpi_unexpected;

	  mpid_recv_handle->mode = T3D_UNEXPECTED_RECV;
	}
	else {
 
#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Didn't find it in the unexpected queue\n");
	  }
#         endif
        }
      }

      /* Is this recv waiting on an unpected msg to complete? */
      if ( mpid_recv_handle->mode == T3D_UNEXPECTED_RECV ) {
	
#       if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	if (DebugFlag) {
	  T3D_Printf("   Checking for completion of unexpected msg\n");
	}
#       endif

	dmpi_unexpected = mpid_recv_handle->dmpi_unexpected;

	if ( dmpi_unexpected->completer <= 0 ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
          if (DebugFlag) {
            T3D_Printf("   unexpected recv is complete\n");
          }
#         endif

          T3D_Copy_unexpected( dmpi_recv_handle );

	  return ( dmpi_recv_handle->completer == 0 );

        }
	else  {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
          if (DebugFlag) {
            T3D_Printf("   unexpected recv is not complete\n");
          }
#         endif

	  return 0;
        } 
      }
	
      if ( dmpi_recv_handle->completer < 0 ) {

	if ( mpid_recv_handle->mode == T3D_PKT_SHORT_SYNC ||
	     mpid_recv_handle->mode == T3D_PKT_LONG_SYNC     ) {
	
	  T3D_CHECK_TARGET( mpid_recv_handle->sync_recv_completed, 8 );

	  shmem_put( (long *)mpid_recv_handle->sync_recv_completed,
		     (long *)&completed,
		     1,
		     mpid_recv_handle->from );

	  T3D_RESET_STACK;

	}

	if ( mpid_recv_handle->needs_copy ) {

#         if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
	  if (DebugFlag) {
	    T3D_Printf("   Copying to misaligned buffer\n");
	    T3D_Printf("      start = 0x%x\n",mpid_recv_handle->start);
	    T3D_Printf("      temp  = 0x%x\n",mpid_recv_handle->temp);
	  }
#         endif

	  memcpy( mpid_recv_handle->start,
		  mpid_recv_handle->temp,
		  mpid_recv_handle->bytes_as_contig );
	  
	  T3D_FREE( mpid_recv_handle->temp );
	}

	DMPI_mark_recv_completed( dmpi_recv_handle );
      }
    }


#  if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_RECV)
   if (DebugFlag) {
     if ( dmpi_recv_handle->completer == 0 )
       T3D_Printf("  Message recv is complete\n");
     else
       T3D_Printf("  Message recv is not complete\n");
   }
#  endif

    return ( dmpi_recv_handle->completer == 0 );
}
