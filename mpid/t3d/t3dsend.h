/*
 *  $Id: t3dsend.h,v 1.3 1995/06/07 06:46:03 bright Exp $
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

#define T3D_MSG_LEN( len ) ( len / 8 ) + ( ( len % 8 ) ? 1 : 0 )

extern T3D_Buf_Status *t3d_dest_bufs;

#endif
