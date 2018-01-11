#ifndef _MPID_BIND
#define _MPID_BIND
/* Bindings for the Device routines */

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

extern void MPID_NX_Abort ANSI_ARGS(( ));
extern void MPID_NX_Myrank ANSI_ARGS(( int * )), 
            MPID_NX_Mysize ANSI_ARGS(( int * )), 
            MPID_NX_End ANSI_ARGS((void));
extern void *MPID_NX_Init ANSI_ARGS(( int *, char *** ));

extern int MPID_NX_post_send ANSI_ARGS(( MPIR_SHANDLE * )), 
           MPID_NX_post_send_sync ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_NX_complete_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_NX_Blocking_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_NX_post_recv ANSI_ARGS(( MPIR_RHANDLE *, int *)),
           MPID_NX_blocking_recv ANSI_ARGS(( MPIR_RHANDLE *)), 
           MPID_NX_complete_recv ANSI_ARGS(( MPIR_RHANDLE *));

extern int MPID_NX_check_device ANSI_ARGS(( int )), 
   MPID_NX_Iprobe ANSI_ARGS(( int, int, int, int *, MPI_Status * )),
   MPID_NX_Probe ANSI_ARGS(( int, int, int, MPI_Status * ));
extern double MPID_NX_Wtime ANSI_ARGS((void));
extern double MPID_NX_Wtick ANSI_ARGS((void));

#ifdef MPID_DEVICE_CODE


extern MPID_Aint MPID_NX_Get_Sync_Id 
    ANSI_ARGS(( MPIR_SHANDLE *, MPID_SHANDLE * ));
extern int MPID_NX_Lookup_SyncAck 
    ANSI_ARGS(( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **));
extern int MPID_SyncAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_SyncReturnAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_Sync_discard ANSI_ARGS(( MPIR_SHANDLE * ));
extern int MPID_NX_Copy_body_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, void * ));
extern int MPID_NX_Copy_body_sync_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_NX_Copy_body_long 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_NX_Copy_body_sync_long
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *pkt, int ));
extern int MPID_NX_Process_unexpected 
    ANSI_ARGS(( MPIR_RHANDLE *, MPIR_RHANDLE *));
extern int MPID_NX_Copy_body 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_NX_Copy_body_unex
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_NX_Ack_Request 
    ANSI_ARGS(( MPID_RHANDLE *, int, MPID_Aint ));
extern int MPID_NX_Complete_Rndv ANSI_ARGS(( MPID_RHANDLE * ));
extern int MPID_NX_Do_Request ANSI_ARGS(( int, int, MPID_Aint ));
extern void MPID_NX_Init_recv_code ANSI_ARGS((void));
extern void MPID_NX_Init_send_code  ANSI_ARGS((void));
extern void MPID_PrintMsgDebug  ANSI_ARGS((void));
extern void MPID_SetSyncDebugFlag  ANSI_ARGS(( void *, int ));

#endif /* MPID_DEVICE_CODE */
#endif /* _MPID_BIND */

