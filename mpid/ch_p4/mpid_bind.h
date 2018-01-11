#ifndef _MPID_BIND
#define _MPID_BIND
/* Bindings for the Device routines */
#ifdef __STDC__
extern void MPID_P4_Abort( );
extern void MPID_P4_Myrank( int * ), MPID_P4_Mysize( int * ), 
            MPID_P4_End(void);
extern void *MPID_P4_Init( int *, char *** );

extern int MPID_P4_post_send( MPIR_SHANDLE * ), 
           MPID_P4_post_send_sync( MPIR_SHANDLE *),
           MPID_P4_complete_send( MPIR_SHANDLE *),
           MPID_P4_Blocking_send( MPIR_SHANDLE *),
           MPID_P4_post_recv( MPIR_RHANDLE *, int *),
           MPID_P4_blocking_recv( MPIR_RHANDLE *), 
           MPID_P4_complete_recv( MPIR_RHANDLE *);

extern int MPID_P4_check_device( int ), 
   MPID_P4_Iprobe( int, int, int, int *, MPI_Status * ),
   MPID_P4_Probe( int, int, int, MPI_Status * );
extern double MPID_P4_Wtime(void);
extern double MPID_P4_Wtick(void);
#else
extern void MPID_P4_Abort();
extern void MPID_P4_Myrank(), MPID_P4_Mysize(), MPID_P4_End();
extern void *MPID_P4_Init();
extern double MPID_P4_Wtime();
extern double MPID_P4_Wtick();
#endif

#ifdef MPID_DEVICE_CODE
#ifdef __STDC__
extern MPID_Aint MPID_P4_Get_Sync_Id( MPIR_SHANDLE *, MPID_SHANDLE * );
extern int MPID_P4_Lookup_SyncAck( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **);
extern int MPID_SyncAck( MPID_Aint, int );
extern void MPID_SyncReturnAck( MPID_Aint, int );
extern void MPID_Sync_discard( MPIR_SHANDLE * );
extern int MPID_P4_Copy_body_short( MPIR_RHANDLE *, MPID_PKT_T *, void * );
extern int MPID_P4_Copy_body_sync_short( MPIR_RHANDLE *, MPID_PKT_T *, int );
extern int MPID_P4_Copy_body_long( MPIR_RHANDLE *, MPID_PKT_T *, int );
extern int MPID_P4_Copy_body_sync_long( MPIR_RHANDLE *, MPID_PKT_T *pkt, int );
extern int MPID_P4_Process_unexpected( MPIR_RHANDLE *, MPIR_RHANDLE *);
extern int MPID_P4_Copy_body( MPIR_RHANDLE *, MPID_PKT_T *, int);
extern int MPID_P4_Copy_body_unex( MPIR_RHANDLE *, MPID_PKT_T *, int);
extern int MPID_P4_Ack_Request( MPID_RHANDLE *, int, MPID_Aint );
extern int MPID_P4_Complete_Rndv( MPID_RHANDLE * );
extern int MPID_P4_Do_Request( int, int, MPID_Aint );
extern void MPID_P4_Init_recv_code(void);
extern void MPID_P4_Init_send_code(void);
extern void MPID_PrintMsgDebug(void);
extern void MPID_SetSyncDebugFlag( void *, int );

#else
extern MPID_Aint MPID_P4_Get_Sync_Id();
extern int MPID_P4_Lookup_SyncAck();
extern int MPID_SyncAck( );
extern void MPID_SyncReturnAck( );
extern void MPID_Sync_discard( );
extern int MPID_P4_Copy_body_short();
extern int MPID_P4_Copy_body_sync_short();
extern int MPID_P4_Copy_body_long();
extern int MPID_P4_Copy_body_sync_long( );
extern int MPID_P4_Process_unexpected();
extern int MPID_P4_Copy_body( );
extern int MPID_P4_Copy_body_unex( );
extern int MPID_P4_Ack_Request( );
extern int MPID_P4_Complete_Rndv( );
extern int MPID_P4_Do_Request( );
extern void MPID_P4_Init_recv_code();
extern void MPID_P4_Init_send_code();
extern void MPID_PrintMsgDebug();
extern void MPID_SetSyncDebugFlag( );
#endif
#endif

#endif

