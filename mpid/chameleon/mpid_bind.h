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

extern void MPID_CH_Abort ANSI_ARGS(( int ));
extern void MPID_CH_Myrank ANSI_ARGS(( int * )), 
            MPID_CH_Mysize ANSI_ARGS(( int * )), 
            MPID_CH_End ANSI_ARGS((void));
extern void MPID_CH_Node_name ANSI_ARGS((char *, int ));
extern void MPID_CH_Version_name ANSI_ARGS((char *));

extern void *MPID_CH_Init ANSI_ARGS(( int *, char *** ));

extern int MPID_CH_post_send ANSI_ARGS(( MPIR_SHANDLE * )), 
           MPID_CH_post_send_sync ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_CH_complete_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_CH_Blocking_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_CH_post_recv ANSI_ARGS(( MPIR_RHANDLE * )),
           MPID_CH_blocking_recv ANSI_ARGS(( MPIR_RHANDLE *)), 
           MPID_CH_complete_recv ANSI_ARGS(( MPIR_RHANDLE *));
extern int MPID_CH_Test_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_CH_Test_recv_push ANSI_ARGS((MPIR_RHANDLE *));

extern void MPID_CH_check_device ANSI_ARGS(( int )), 
   MPID_CH_Probe ANSI_ARGS(( int, int, int, MPI_Status * ));
extern int MPID_CH_Iprobe ANSI_ARGS(( int, int, int, int *, MPI_Status * ));
extern int MPID_CH_Cancel ANSI_ARGS((MPIR_COMMON *)); 
extern int MPID_CH_check_incoming ANSI_ARGS((MPID_BLOCKING_TYPE));

extern MPID_CH_Comm_init ANSI_ARGS((MPI_Comm, MPI_Comm));
extern void MPID_SetSpaceDebugFlag ANSI_ARGS(( int ));
extern void MPID_SetDebugFile ANSI_ARGS(( char * ));

extern int MPID_CH_init_hetero ANSI_ARGS(( int *, char *** ));
extern int MPID_CH_Dest_byte_order ANSI_ARGS(( int ));
extern int MPID_CH_Comm_msgrep ANSI_ARGS(( MPI_Comm ));

#ifndef MPID_CH_Wtime
extern double MPID_CH_Wtime ANSI_ARGS((void));
#endif
extern double MPID_CH_Wtick ANSI_ARGS((void));

#ifdef MPID_DEVICE_CODE


extern MPID_Aint MPID_CH_Get_Sync_Id 
    ANSI_ARGS(( MPIR_SHANDLE *, MPID_SHANDLE * ));
extern int MPID_CH_Lookup_SyncAck 
    ANSI_ARGS(( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **));
extern int MPID_SyncAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_SyncReturnAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_Sync_discard ANSI_ARGS(( MPIR_SHANDLE * ));

extern void MPID_CH_Pkt_pack ANSI_ARGS(( MPID_PKT_T *, int, int ));
extern void MPID_CH_Pkt_unpack ANSI_ARGS(( MPID_PKT_T *, int, int ));

extern void MPID_CH_Print_Send_Handle ANSI_ARGS(( MPIR_SHANDLE *));

/*
These are now static ...
extern int MPID_CH_Copy_body_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, void * ));
extern int MPID_CH_Copy_body_sync_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_CH_Copy_body_long 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_CH_Copy_body_sync_long
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *pkt, int ));
 */
extern int MPID_CH_Process_unexpected 
    ANSI_ARGS(( MPIR_RHANDLE *, MPIR_RHANDLE *));
extern int MPID_CH_Copy_body 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_CH_Copy_body_unex
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_CH_Ack_Request 
    ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_Aint, int ));
extern int MPID_CH_Complete_Rndv ANSI_ARGS(( MPID_RHANDLE * ));
extern int MPID_CH_Do_Request ANSI_ARGS(( int, int, MPID_Aint ));
extern void MPID_CH_Init_recv_code ANSI_ARGS((void));
extern void MPID_CH_Init_send_code  ANSI_ARGS((void));
extern void MPID_PrintMsgDebug  ANSI_ARGS((void));
extern void MPID_SetSyncDebugFlag  ANSI_ARGS(( void *, int ));

/* These are defined only for if MPID_USE_GET is */
extern void * MPID_SetupGetAddress ANSI_ARGS(( void *, int *, int ));
extern void   MPID_FreeGetAddress ANSI_ARGS(( void * ));

extern int MPID_CH_Do_get ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_PKT_GET_T * ));

extern void MPID_CH_Get_print_pkt ANSI_ARGS((FILE *, MPID_PKT_T *));
extern void MPID_CH_Print_pkt_data ANSI_ARGS((char *, char *, int ));

extern int MPID_CH_Hetero_free ANSI_ARGS((void));

#endif /* MPID_DEVICE_CODE */
#endif /* _MPID_BIND */






