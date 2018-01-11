/*
 *  $Id: mpid_bind.h,v 1.3 1995/06/07 06:14:24 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _DIMPID_BIND_INCLUDED
#define _DIMPID_BIND_INCLUDED

/*
 * ADI ansi declarations
 */

/* Prototypes for t3dcoll.c */
int T3D_Comm_init( MPI_Comm comm , MPI_Comm newcomm );
int T3D_Comm_free( MPI_Comm comm );
void T3D_Barrier(  MPI_Comm comm );
void T3D_Reduce_sum_int( int *sendbuf , int *recvbuf , MPI_Comm comm );
void T3D_Reduce_sum_double( double *sendbuf , double *recvbuf , MPI_Comm comm );
int T3D_Comm_init( MPI_Comm comm , MPI_Comm newcomm );
int T3D_Comm_free( MPI_Comm comm );
void T3D_Barrier(  MPI_Comm comm );
void T3D_Reduce_sum_int( int *sendbuf , int *recvbuf , MPI_Comm comm );
void T3D_Reduce_sum_double( double *sendbuf , double *recvbuf , MPI_Comm comm );
int T3D_Comm_init( MPI_Comm comm , MPI_Comm newcomm );
int T3D_Comm_free( MPI_Comm comm );
void T3D_Barrier(  MPI_Comm comm );
void T3D_Reduce_sum_int( int *sendbuf , int *recvbuf , MPI_Comm comm );
void T3D_Reduce_sum_double( double *sendbuf , double *recvbuf , MPI_Comm comm );

/* Prototypes for t3ddebug.c -- no prototypes for the moment */
void T3D_Error(  );
void T3D_Printf(  );

/* Prototypes for t3ddevice.c */
void T3D_Set_device_debug_flag( int flag );

/* Prototypes for t3devent.c */
int T3D_Check_device( );
int T3D_Cancel( MPIR_COMMON *r );

/* Prototypes for t3dinit.c */
void T3D_Set_init_debug_flag( int flag );
void T3D_Set_space_debug_flag( int flag );
void T3D_Myrank( int *rank );
void T3D_Mysize( int *size );
int T3D_Id( );
double T3D_Wtime( );
double T3D_Wtick( );
void T3D_Initenv( int *argc , char **argv );
void *T3D_Init( int *argc , char ***argv );
void T3D_Abort( int code );
void T3D_End( );
void T3D_Node_name( char *name , int len );
void T3D_Version_name( char *name );

/* Prototypes for t3dprobe.c */
int T3D_Iprobe( int tag , int source , int context_id , int *found , MPI_Status *status );
int T3D_Probe( int tag , int source , int context_id , MPI_Status *status );

/* Prototypes for t3drecv.c */
void T3D_Set_debug_flag( int f );
void T3D_Set_recv_debug_flag( int f );
void T3D_Set_msg_debug_flag( int f );
int T3D_Get_msg_debug_flag( );
void T3D_Print_msg_debug( );
int T3D_Recv_packet ( T3D_PKT_T **pkt );
void T3D_Reuse_buf( int buf );
void T3D_Init_recv_code( );
int T3D_Process_unexpected( MPIR_RHANDLE *dmpi_recv_handle , MPIR_RHANDLE *dmpi_unexpected );
int T3D_Copy_body( MPIR_RHANDLE *dmpi_recv_handle , int from, T3D_PKT_T *pkt );
int T3D_Copy_body_unex( MPIR_RHANDLE *dmpi_recv_handle , int from, T3D_PKT_T *pkt );
int T3D_Check_incoming( int blocking );
int T3D_Post_recv(  MPIR_RHANDLE *dmpi_recv_handle );
int T3D_Complete_recv(  MPIR_RHANDLE *dmpi_recv_handle );
int T3D_Blocking_recv(  MPIR_RHANDLE *dmpi_recv_handle );
int T3D_Test_recv( MPIR_RHANDLE *dmpi_send_handle );

/* Prototypes for t3dsend.c */
void T3D_Set_send_debug_flag( int f );
int T3D_Post_send_sync(  MPIR_SHANDLE *dmpi_send_handle );
int T3D_Post_send_ready(  MPIR_SHANDLE *dmpi_send_handle );
int T3D_Post_send_short( MPIR_SHANDLE *dmpi_send_handle , MPID_SHANDLE *mpid_send_handle , int len );
int T3D_Post_send_very_short( MPIR_SHANDLE *dmpi_send_handle , MPID_SHANDLE *mpid_send_handle , int len );
int T3D_Post_send(  MPIR_SHANDLE *dmpi_send_handle );
int T3D_Blocking_send( MPIR_SHANDLE *dmpi_send_handle );
int T3D_Complete_send(  MPIR_SHANDLE *dmpi_send_handle );
int T3D_Test_send( MPIR_SHANDLE *dmpi_send_handle );

/* Prototypes for t3dsync.c */
void T3D_Set_sync_debug_flag( int f );

#endif
