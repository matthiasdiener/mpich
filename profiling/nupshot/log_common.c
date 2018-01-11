/*
   Logfile stuff common to all formats for Upshot.
*/

#include "log_common.h"

/* Sorry about this kludge, but our ANSI C compiler on our suns has broken
   header files */
#if defined(sparc) && defined(__STDC__)
int printf( const char*, ... );
#endif


int LogFormatError( filename, line )
char *filename;
int line;
{
  printf( "Logfile format error in %s, line %d.\n", filename, line );
  return 0;
}


double Log_StartTime( log )
logData *log;
{
  return log->starttime;
}

double Log_EndTime( log )
logData *log;
{
  return log->endtime;
}

int Log_Np( log )
logData *log;
{
  return log->np;
}

int Log_Loaded( log )
logData *log;
{
  return log->loaded;
}

