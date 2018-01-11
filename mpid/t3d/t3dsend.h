/*
 *  $Id: t3dsend.h,v 1.4 1995/09/15 02:00:48 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DSEND_INCLUDED
#define _T3DSEND_INCLUDED


/***************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ***************************************************************************/
#define MPID_Set_send_debug_flag(ctx,f)   T3D_Set_send_debug_flag(f)
#define MPID_Post_send(ctx,shandle)       T3D_Post_send(shandle) 
#define MPID_Post_send_sync(ctx,shandle)  T3D_Post_send_sync(shandle) 
#define MPID_Post_send_ready(ctx,shandle) T3D_Post_send(shandle) 
#define MPID_Blocking_send(ctx,shandle)   T3D_Blocking_send(shandle)
#define MPID_Blocking_send_ready(ctx,shandle) \
        MPID_Blocking_send(ctx,shandle)
#define MPID_Complete_send(ctx,shandle)   T3D_Complete_send(shandle)
#define MPID_Test_send(ctx,shandle)       T3D_Test_send(shandle)

#define T3D_MSG_LEN_64( len ) ( len / 8 ) + ( ( len % 8 ) ? 1 : 0 )
#define T3D_MSG_LEN_32( len ) ( len / 4 ) + ( ( len % 4 ) ? 1 : 0 )

#define T3D_LONG_SEND( target, source, len, dest )                          \
                                                                            \
  if ( ((int)target & 0x7) || ((int)source & 0x7)) {                        \
    /* Do 32-bit aligned send */                                            \
    shmem_put32( (short *)target,                                           \
		         (short *)source,                                   \
		T3D_MSG_LEN_32( len ),                                      \
		dest );                                                     \
  }                                                                         \
  else {                                                                    \
    /* Do 64-bit aligned send */                                            \
	       shmem_put( (long *)target,                                   \
			 (long *)source,                                    \
			 T3D_MSG_LEN_64( len ),                             \
			 dest );                                            \
   }

extern T3D_Buf_Status *t3d_dest_bufs;

#endif
