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

extern void MPID_N3_Abort ANSI_ARGS(( ));
extern void MPID_N3_Myrank ANSI_ARGS(( int * )), 
            MPID_N3_Mysize ANSI_ARGS(( int * )), 
            MPID_N3_End ANSI_ARGS((void));
extern void *MPID_N3_Init ANSI_ARGS(( int *, char *** ));

extern int MPID_N3_post_send ANSI_ARGS(( MPIR_SHANDLE * )), 
           MPID_N3_post_send_sync ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_N3_complete_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_N3_Blocking_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_N3_post_recv ANSI_ARGS(( MPIR_RHANDLE *, int *)),
           MPID_N3_blocking_recv ANSI_ARGS(( MPIR_RHANDLE *)), 
           MPID_N3_complete_recv ANSI_ARGS(( MPIR_RHANDLE *));

extern int MPID_N3_check_device ANSI_ARGS(( int )), 
   MPID_N3_Iprobe ANSI_ARGS(( int, int, int, int *, MPI_Status * )),
   MPID_N3_Probe ANSI_ARGS(( int, int, int, MPI_Status * ));
extern double MPID_N3_Wtime ANSI_ARGS((void));
extern double MPID_N3_Wtick ANSI_ARGS((void));

#ifdef MPID_DEVICE_CODE


extern MPID_Aint MPID_N3_Get_Sync_Id 
    ANSI_ARGS(( MPIR_SHANDLE *, MPID_SHANDLE * ));
extern int MPID_N3_Lookup_SyncAck 
    ANSI_ARGS(( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **));
extern int MPID_SyncAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_SyncReturnAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_Sync_discard ANSI_ARGS(( MPIR_SHANDLE * ));
extern int MPID_N3_Copy_body_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, void * ));
extern int MPID_N3_Copy_body_sync_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_N3_Copy_body_long 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_N3_Copy_body_sync_long
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *pkt, int ));
extern int MPID_N3_Process_unexpected 
    ANSI_ARGS(( MPIR_RHANDLE *, MPIR_RHANDLE *));
extern int MPID_N3_Copy_body 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_N3_Copy_body_unex
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_N3_Ack_Request 
    ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_Aint, int ));
extern int MPID_N3_Complete_Rndv ANSI_ARGS(( MPID_RHANDLE * ));
extern int MPID_N3_Do_Request ANSI_ARGS(( int, int, MPID_Aint ));
extern void MPID_N3_Init_recv_code ANSI_ARGS((void));
extern void MPID_N3_Init_send_code  ANSI_ARGS((void));
extern void MPID_PrintMsgDebug  ANSI_ARGS((void));
extern void MPID_SetSyncDebugFlag  ANSI_ARGS(( void *, int ));

/* These are defined only for if MPID_USE_GET is */
extern void * MPID_SetupGetAddress ANSI_ARGS(( void *, int *, int ));
extern void   MPID_FreeGetAddress ANSI_ARGS(( void * ));

extern int MPID_N3_Do_get ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_PKT_GET_T * ));

#endif /* MPID_DEVICE_CODE */
#endif /* _MPID_BIND */






