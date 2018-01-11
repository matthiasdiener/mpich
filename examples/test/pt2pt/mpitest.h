#ifndef MPITEST_TEST
#define MPITEST_TEST

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

void Test_Init ANSI_ARGS(( char *, int ));
#if defined(__STDC__)
void Test_Printf(char *, ...);
#else
void Test_Printf();
#endif
void Test_Message ANSI_ARGS((char *));
void Test_Failed ANSI_ARGS((char *));
void Test_Passed ANSI_ARGS((char *));
int Summarize_Test_Results ANSI_ARGS((void));
void Test_Finalize ANSI_ARGS((void));
void Test_Waitforall ANSI_ARGS((void));
#endif
