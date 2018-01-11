#ifndef MPID_Wtime

/* Special features of timer for NX */

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

extern double MPID_CH_Wtime();
extern double MPID_CH_Wtick();

#define MPID_Wtime(t) *(t) = MPID_CH_Wtime()
#define MPID_Wtick(t) *(t) = MPID_CH_Wtick()
#define MPID_Wtime_is_global MPID_Time_is_global

extern int MPID_Time_is_global ANSI_ARGS((void));

#endif
