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


logData *Log_OpenData()
{
  logData *log;

  log = (logData*) malloc( sizeof( logData ) );
  if (!log) {
    fprintf( stderr, "Out of memory opening log.\n" );
  }

  log->is_reading = 0;
  log->halt_reading = 0;
  log->loaded = 0;

  return log;
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

int Log_Halt( log )
logData *log;
{
  log->halt_reading = 1;
  return 0;
}

int Log_Halted( log )
logData *log;
{
  return log->halt_reading;
}


int Log_CloseData( log )
logData *log;
{
  Event_Close( log->events );
  State_Close( log->states );
  Msg_Close( log->msgs );

  free( (void*)log );
  return 0;
}

