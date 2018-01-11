/* mpe_log.c */
/* Custom Fortran interface file */
/* These have been edited because they require special string processing */
#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif
#include <stdio.h>
#include "mpe.h"
extern char *malloc();

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef FORTRANCAPS
#define mpe_init_log_ MPE_INIT_LOG
#define mpe_start_log_ MPE_START_LOG
#define mpe_stop_log_ MPE_STOP_LOG
#define mpe_describe_state_ MPE_DESCRIBE_STATE
#define mpe_describe_event_ MPE_DESCRIBE_EVENT
#define mpe_log_event_ MPE_LOG_EVENT
#define mpe_finish_log MPE_FINISH_LOG
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_init_log_ mpe_init_log__
#define mpe_start_log_ mpe_start_log__
#define mpe_stop_log_ mpe_stop_log__
#define mpe_describe_state_ mpe_describe_state__
#define mpe_describe_event_ mpe_describe_event__
#define mpe_log_event_ mpe_log_event__
#define mpe_finish_log mpe_finish_log__
#elif defined(FORTRANNOUNDERSCORE)
#define mpe_init_log_ mpe_init_log
#define mpe_start_log_ mpe_start_log
#define mpe_stop_log_ mpe_stop_log
#define mpe_describe_state_ mpe_describe_state
#define mpe_describe_event_ mpe_describe_event
#define mpe_log_event_ mpe_log_event
#define mpe_finish_log_ mpe_finish_log
#endif

/* 
   This function makes a copy of a Fortran string into a C string.  Some
   Unix Fortran compilers add nulls at the ends of string CONSTANTS, but
   (a) not for substring expressions and (b) not all compilers do so (e.g.,
   RS6000)
 */
static char *mpe_tmp_cpy( s, d )
char *s;
int  d;
{
char *p;
p = (char *)malloc( d + 1 );
if (!p) ;
strncpy( p, s, d );
p[d] = 0;
return p;
}

 int  mpe_init_log_()
{
return MPE_Init_log();
}
 int  mpe_start_log_()
{
return MPE_Start_log();
}
 int  mpe_stop_log_()
{
return MPE_Stop_log();
}
 int  mpe_describe_state_( start, end, name, color, d1, d2 )
int *start, *end;
char *name, *color;
int  d1, d2;
{
char *c1, *c2;
int  err;
c1 = mpe_tmp_cpy( name, d1 );
c2 = mpe_tmp_cpy( color, d2 );
err = MPE_Describe_state(*start,*end,c1, c2);
free( c1 );
free( c2 );
return err;
}
 int  mpe_describe_event_( event, name, d1)
int *event;
char *name;
int  d1;
{
char *c1;
int  err;
c1 = mpe_tmp_cpy( name, d1 );
err = MPE_Describe_event(*event,c1);
free( c1 );
return err;
}
 int  mpe_log_event_(event,data,string, d1)
int *event, *data;
char *string;
int  d1;
{
char *c1;
int  err;
c1 = mpe_tmp_cpy( string, d1 );
err = MPE_Log_event(*event,*data,c1);
free( c1 );
return err;
}
 int  mpe_finish_log_( filename, d1)
char *filename;
int  d1;
{
char *c1;
int  err;
c1 = mpe_tmp_cpy( filename, d1 );
err =  MPE_Finish_log(c1);
free( c1 );
return err;
}
