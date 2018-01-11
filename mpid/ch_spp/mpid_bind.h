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

extern void MPID_SPP_Abort ANSI_ARGS(( ));
extern void MPID_SPP_Myrank ANSI_ARGS(( int * )), 
            MPID_SPP_Mysize ANSI_ARGS(( int * )), 
            MPID_SPP_End ANSI_ARGS((void));
extern void MPID_SPP_Node_name ANSI_ARGS((char *, int ));
extern void MPID_SPP_Version_name ANSI_ARGS((char *));

extern void *MPID_SPP_Init ANSI_ARGS(( int *, char *** ));

extern int MPID_SPP_post_send ANSI_ARGS(( MPIR_SHANDLE * )), 
           MPID_SPP_post_send_sync ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_SPP_complete_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_SPP_Blocking_send ANSI_ARGS(( MPIR_SHANDLE *)),
           MPID_SPP_post_recv ANSI_ARGS(( MPIR_RHANDLE * )),
           MPID_SPP_blocking_recv ANSI_ARGS(( MPIR_RHANDLE *)), 
           MPID_SPP_complete_recv ANSI_ARGS(( MPIR_RHANDLE *));
extern void MPID_SPP_check_device ANSI_ARGS(( int )), 
   MPID_SPP_Probe ANSI_ARGS(( int, int, int, MPI_Status * ));
extern int MPID_SPP_Iprobe ANSI_ARGS(( int, int, int, int *, MPI_Status * ));
extern int MPID_SPP_Cancel ANSI_ARGS((MPIR_COMMON *)); 
extern int MPID_SPP_check_incoming ANSI_ARGS((MPID_BLOCKING_TYPE));

extern void MPID_SetSpaceDebugFlag ANSI_ARGS(( int ));
extern void MPID_SetDebugFile ANSI_ARGS(( char * ));

#ifndef MPID_SPP_Wtime
extern double MPID_SPP_Wtime ANSI_ARGS((void));
#endif
extern double MPID_SPP_Wtick ANSI_ARGS((void));

#ifdef MPID_DEVICE_CODE


extern MPID_Aint MPID_SPP_Get_Sync_Id 
    ANSI_ARGS(( MPIR_SHANDLE *, MPID_SHANDLE * ));
extern int MPID_SPP_Lookup_SyncAck 
    ANSI_ARGS(( MPID_Aint, MPIR_SHANDLE **,MPID_SHANDLE **));
extern int MPID_SyncAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_SyncReturnAck ANSI_ARGS(( MPID_Aint, int ));
extern void MPID_Sync_discard ANSI_ARGS(( MPIR_SHANDLE * ));

extern void MPID_SPP_Pkt_pack ANSI_ARGS(( MPID_PKT_T *, int, int ));
extern void MPID_SPP_Pkt_unpack ANSI_ARGS(( MPID_PKT_T *, int, int ));

/*
These are now static ...
extern int MPID_SPP_Copy_body_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, void * ));
extern int MPID_SPP_Copy_body_sync_short 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_SPP_Copy_body_long 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int ));
extern int MPID_SPP_Copy_body_sync_long
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *pkt, int ));
 */
extern int MPID_SPP_Process_unexpected 
    ANSI_ARGS(( MPIR_RHANDLE *, MPIR_RHANDLE *));
extern int MPID_SPP_Copy_body 
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_SPP_Copy_body_unex
    ANSI_ARGS(( MPIR_RHANDLE *, MPID_PKT_T *, int));
extern int MPID_SPP_Ack_Request 
    ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_Aint, int ));
extern int MPID_SPP_Complete_Rndv ANSI_ARGS(( MPID_RHANDLE * ));
extern int MPID_SPP_Do_Request ANSI_ARGS(( int, int, MPID_Aint ));
extern void MPID_SPP_Init_recv_code ANSI_ARGS((void));
extern void MPID_SPP_Init_send_code  ANSI_ARGS((void));
extern void MPID_PrintMsgDebug  ANSI_ARGS((void));
extern void MPID_SetSyncDebugFlag  ANSI_ARGS(( void *, int ));

/* These are defined only for if MPID_USE_GET is */
extern void * MPID_SetupGetAddress ANSI_ARGS(( void *, int *, int ));
extern void   MPID_FreeGetAddress ANSI_ARGS(( void * ));

extern int MPID_SPP_Do_get ANSI_ARGS(( MPIR_RHANDLE *, int, MPID_PKT_GET_T * ));

extern void MPID_SPP_Get_print_pkt ANSI_ARGS((FILE *, MPID_PKT_T *));
extern void MPID_SPP_Print_pkt_data ANSI_ARGS((char *, char *, int ));

#endif /* MPID_DEVICE_CODE */
#endif /* _MPID_BIND */






