#ifndef _MPID_BIND
#define _MPID_BIND
/* Bindings for the Device routines */
#ifdef __STDC__
extern void MPID_CH_Abort( );
extern void MPID_CH_Myrank( int * ), MPID_CH_Mysize( int * ), 
            MPID_CH_End(void);
extern void *MPID_CH_Init( int *, char *** );

extern int MPID_CH_post_send( MPIR_SHANDLE * ), 
           MPID_CH_post_send_sync( MPIR_SHANDLE *),
           MPID_CH_complete_send( MPIR_SHANDLE *),
           MPID_CH_Blocking_send( MPIR_SHANDLE *),
           MPID_CH_post_recv( MPIR_RHANDLE *, int *),
           MPID_CH_blocking_recv( MPIR_RHANDLE *), 
           MPID_CH_complete_recv( MPIR_RHANDLE *);

extern int MPID_CH_check_device( int ), 
   MPID_CH_Iprobe( int, int, int, int *, MPI_Status * ),
   MPID_CH_Probe( int, int, int, MPI_Status * );
extern double MPID_CH_Wtime(void);
extern double MPID_CH_Wtick(void);
#else
extern void MPID_CH_Abort();
extern void MPID_CH_Myrank(), MPID_CH_Mysize(), MPID_CH_End();
extern void *MPID_CH_Init();
extern double MPID_CH_Wtime();
extern double MPID_CH_Wtick();
#endif

#ifdef MPID_DEVICE_CODE
#ifdef __STDC__
extern MPID_Aint MPID_CH_Get_Sync_Id( MPIR_SHANDLE *, MPID_SHANDLE * );
extern int MPID_CH_Lookup_SyncAck( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **);
extern int MPID_SyncAck( MPID_Aint, int );
extern void MPID_SyncReturnAck( MPID_Aint, int );
extern void MPID_Sync_discard( MPIR_SHANDLE * );
extern int MPID_CH_Copy_body_short( MPIR_RHANDLE *, MPID_PKT_T *, void * );
extern int MPID_CH_Copy_body_sync_short( MPIR_RHANDLE *, MPID_PKT_T *, int );
extern int MPID_CH_Copy_body_long( MPIR_RHANDLE *, MPID_PKT_T *, int );
extern int MPID_CH_Copy_body_sync_long( MPIR_RHANDLE *, MPID_PKT_T *pkt, int );
extern int MPID_CH_Process_unexpected( MPIR_RHANDLE *, MPIR_RHANDLE *);
extern int MPID_CH_Copy_body( MPIR_RHANDLE *, MPID_PKT_T *, int);
extern int MPID_CH_Copy_body_unex( MPIR_RHANDLE *, MPID_PKT_T *, int);
extern int MPID_CH_Ack_Request( MPID_RHANDLE *, int, MPID_Aint );
extern int MPID_CH_Complete_Rndv( MPID_RHANDLE * );
extern int MPID_CH_Do_Request( int, int, MPID_Aint );
extern void MPID_CH_Init_recv_code(void);
extern void MPID_CH_Init_send_code(void);
extern void MPID_PrintMsgDebug(void);
extern void MPID_SetSyncDebugFlag( void *, int );

#else
extern MPID_Aint MPID_CH_Get_Sync_Id();
extern int MPID_CH_Lookup_SyncAck();
extern int MPID_SyncAck( );
extern void MPID_SyncReturnAck( );
extern void MPID_Sync_discard( );
extern int MPID_CH_Copy_body_short();
extern int MPID_CH_Copy_body_sync_short();
extern int MPID_CH_Copy_body_long();
extern int MPID_CH_Copy_body_sync_long( );
extern int MPID_CH_Process_unexpected();
extern int MPID_CH_Copy_body( );
extern int MPID_CH_Copy_body_unex( );
extern int MPID_CH_Ack_Request( );
extern int MPID_CH_Complete_Rndv( );
extern int MPID_CH_Do_Request( );
extern void MPID_CH_Init_recv_code();
extern void MPID_CH_Init_send_code();
extern void MPID_PrintMsgDebug();
extern void MPID_SetSyncDebugFlag( );
#endif
#endif

#endif

