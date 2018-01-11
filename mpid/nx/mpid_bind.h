#ifndef _MPID_BIND
#define _MPID_BIND
/* Bindings for the Device routines 
 * These for the Intel Paragon versions.
 */

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif

#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

extern void MPID_NX_Abort  ANSI_ARGS((int ));
extern void MPID_NX_Myrank ANSI_ARGS((int * ));
extern void MPID_NX_Mysize ANSI_ARGS((int * ));
extern void MPID_NX_End    ANSI_ARGS((void));
extern void *MPID_NX_Init  ANSI_ARGS((int *, char *** ));

extern int MPID_NX_Post_send     ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_NX_Complete_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_NX_Blocking_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_NX_Post_recv     ANSI_ARGS((MPIR_RHANDLE *));
extern int MPID_NX_Blocking_recv ANSI_ARGS((MPIR_RHANDLE *)); 
extern int MPID_NX_Complete_recv ANSI_ARGS((MPIR_RHANDLE *));

extern int MPID_NX_Test_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_NX_Test_recv ANSI_ARGS((MPIR_RHANDLE *));

extern int MPID_NX_Iprobe   ANSI_ARGS((int, int, int, int *, MPI_Status * ));
extern int MPID_NX_Probe    ANSI_ARGS((int, int, int, MPI_Status * ));

extern double MPID_NX_Wtime ANSI_ARGS((void));
extern double MPID_NX_Wtick ANSI_ARGS((void));

#endif /* _MPID_BIND */






