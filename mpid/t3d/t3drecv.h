/*
 *  $Id: t3drecv.h,v 1.5 1995/06/07 06:45:52 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DRECV_INCLUDED
#define _T3DRECV_INCLUDED

/******************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ******************************************************************************/
#define MPID_Set_send_flag(ctx,f)           T3D_Set_send_flag(f)
#define MPID_Set_recv_debug_flag(ctx,f)     T3D_Set_recv_debug_flag(f)
#define MPID_Set_msg_debug_flag(ctx,f)      T3D_Set_msg_debug_flag(f)
#define MPID_Get_msg_debug_flag             T3D_Get_msg_debug_flag
#define MPID_Print_msg_debug                T3D_Print_msg_debug
#define MPID_Init_recv_code                 T3D_Init_recv_code
#define MPID_Post_recv(ctx,rhandle)         T3D_Post_recv(rhandle) 
#define MPID_Complete_recv(ctx,rhandle)     T3D_Complete_recv(rhandle) 
#define MPID_Blocking_recv(ctx,rhandle)     T3D_Blocking_recv(rhandle) 
#define MPID_Test_recv(ctx,rhandle)         T3D_Test_recv(rhandle)

/****************************************************************************
  Packet definitions
 ***************************************************************************/

#define T3D_BUFFER_LENGTH  32

typedef enum { T3D_BUF_AVAIL=0, T3D_BUF_IN_USE=1 } T3D_Buf_Status;

typedef enum { T3D_PKT_SHORT      = 0,
	       T3D_PKT_LONG       = 1,
	       T3D_PKT_SHORT_SYNC = 2,
	       T3D_PKT_LONG_SYNC  = 3 
} T3D_Pkt_t;

typedef struct {
  T3D_Buf_Status  status;
  T3D_Pkt_t       mode;
  int             context_id;
  int             lrank;
  int             tag;
  int             len;
} T3D_PKT_HEAD_T;

typedef struct {
  T3D_Buf_Status  status;
  T3D_Pkt_t       mode;
  int             context_id;
  int             lrank;
  int             tag;
  int             len;
  char            buffer[T3D_BUFFER_LENGTH];
} T3D_PKT_SHORT_T;

typedef struct {
  T3D_Buf_Status  status;
  T3D_Pkt_t       mode;
  int             context_id;
  int             lrank;
  int             tag;
  int             len;
  char           *buffer;     /* location of where to send remote buffer pointer,
                                 remote completed flag, and local completed flag */
} T3D_PKT_LONG_T;

typedef struct {
  T3D_Buf_Status  status;
  T3D_Pkt_t       mode;
  int             context_id;
  int             lrank;
  int             tag;
  int             len;
  char           *local_send_completed;
  char            buffer[T3D_BUFFER_LENGTH];
} T3D_PKT_SHORT_SYNC_T;

typedef struct {
  T3D_Buf_Status  status;
  T3D_Pkt_t       mode;
  int             context_id;
  int             lrank;
  int             tag;
  int             len;
  char           *local_send_completed;
  char           *buffer;     /* location of where to send remote buffer pointer,
                                 remote completed flag, and local completed flag */
} T3D_PKT_LONG_SYNC_T;

typedef union {
  T3D_PKT_HEAD_T       head;
  T3D_PKT_SHORT_T      short_pkt;
  T3D_PKT_LONG_T       long_pkt;
  T3D_PKT_SHORT_SYNC_T short_sync_pkt;
  T3D_PKT_LONG_SYNC_T  long_sync_pkt;
} T3D_PKT_T;

extern T3D_PKT_T  *t3d_recv_bufs;

#endif


