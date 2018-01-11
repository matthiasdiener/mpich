#ifndef _MPID_BIND
#define _MPID_BIND
/* Bindings for the Device routines 
 * These for the MEIKO CS2 versions.
 */

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif

#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

extern void MPID_MEIKO_Abort  ANSI_ARGS((int ));
extern void MPID_MEIKO_Myrank ANSI_ARGS((int * ));
extern void MPID_MEIKO_Mysize ANSI_ARGS((int * ));
extern void MPID_MEIKO_End    ANSI_ARGS((void));
extern void *MPID_MEIKO_Init  ANSI_ARGS((int *, char *** ));

extern int MPID_MEIKO_post_send     ANSI_ARGS((MPIR_SHANDLE *, int ));
extern int MPID_MEIKO_complete_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_MEIKO_Blocking_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_MEIKO_post_recv     ANSI_ARGS((MPIR_RHANDLE *));
extern int MPID_MEIKO_blocking_recv ANSI_ARGS((MPIR_RHANDLE *)); 
extern int MPID_MEIKO_complete_recv ANSI_ARGS((MPIR_RHANDLE *));

extern int MPID_MEIKO_Test_send ANSI_ARGS((MPIR_SHANDLE *));
extern int MPID_MEIKO_Test_recv ANSI_ARGS((MPIR_RHANDLE *));

extern int MPID_MEIKO_Iprobe   ANSI_ARGS((int, int, int, int *, MPI_Status * ));
extern int MPID_MEIKO_Probe    ANSI_ARGS((int, int, int, MPI_Status * ));

extern double MPID_MEIKO_Wtime ANSI_ARGS((void));
extern double MPID_MEIKO_Wtick ANSI_ARGS((void));

#endif /* _MPID_BIND */






