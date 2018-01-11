/*
   Format-unspecific logfile stuff for Upshot.
*/

#include <stdio.h>
#include "log.h"



#define DEBUG 0


int PreProcessLog( filename, log_data, log_file )
char *filename;
logData *log_data;
logFile *log_file;
{
  log_data->loaded = 0;
  switch (log_file->format) {
  case alog_format:
    return AlogPreProcessLog( filename, log_data, (alogData *)log_file );
  case blog_format:
  case old_picl_format:
  case new_picl_format:
  case sddf_format:
  case vt_format:
  case unknown_format:
    break;
  }

  fprintf( stderr, "Unknown logfile format.\n" );
  return -1;
}



int ProcessLog( log_data, log_file )
logData *log_data;
logFile *log_file;
{
#if DEBUG
  fprintf( stderr, "Loading file.\n" );
#endif
  log_data->loaded = 1;

  switch (log_file->format) {
  case alog_format:
    return AlogProcessLog( log_data, (alogData *)log_file );
  case blog_format:
  case old_picl_format:
  case new_picl_format:
  case sddf_format:
  case vt_format:
  case unknown_format:
    break;
  }

  fprintf( stderr, "Unknown logfile format.\n" );
  return -1;
}



int CloseLog( log_data, log_file )
logData *log_data;
logFile *log_file;
{
  switch (log_file->format) {
  case alog_format:
    return AlogCloseLog( log_data, (alogData *)log_file );
  case blog_format:
  case old_picl_format:
  case new_picl_format:
  case sddf_format:
  case vt_format:
  case unknown_format:
    break;
  }

  fprintf( stderr, "Unknown logfile format.\n" );
  return -1;
}



  /* Event-handling stuff */

int Log_NeventDefs( log )
logData *log;
{
  return Event_Ndefs( log->events );
}

int Log_GetEventDef( log, def_num, name )
logData *log;
int def_num;
char **name;
{
  return Event_GetDef( log->events, def_num, name );
}

int Log_SetEventDef( log, def_num, name )
logData *log;
int def_num;
char *name;
{
  return Event_SetDef( log->events, def_num, name );
}

int Log_Nevents( log )
logData *log;
{
  return Event_N( log->events );
}

int Log_GetEvent( log, event_num, type, proc, time )
logData *log;
int event_num, *type, *proc;
double *time;
{
  return Event_Get( log->events, event_num, type, proc, time );
}


  /* State-handling stuff */

int Log_NstateDefs( log )
logData *log;
{
  return State_Ndefs( log->states );
}

int Log_GetStateDef( log, def_num, name, color, bitmap )
logData *log;
int def_num;
char **name, **color, **bitmap;
{
  return State_GetDef( log->states, def_num, name, color, bitmap );
}

int Log_SetStateDef( log, def_num, name, color, bitmap )
logData *log;
int def_num;
char *name, *color, *bitmap;
{
  return State_SetDef( log->states, def_num, name, color, bitmap );
}

int Log_Nstates( log )
logData *log;
{
  return State_N( log->states );
}

int Log_GetState( log, state_num, type, proc, startTime, endTime,
		  parent, firstChild, overlapLevel )
logData *log;
int state_num, *type, *proc, *parent, *firstChild, *overlapLevel;
double *startTime, *endTime;
{
  return State_Get( log->states, state_num, type, proc, startTime, endTime,
		    parent, firstChild, overlapLevel );
}

void *Log_AddDrawState( log, fn, data )
logData *log;
drawStateFn *fn;
void *data;
{
  return State_AddDraw( log->states, fn, data );
}

int Log_RmDrawState( log, token )
logData *log;
void *token;
{
  return State_RmDraw( log->states, token );
}



int Log_GetVisStates( log, time1, time2, list1, n1, list2, n2 )
logData *log;
double time1, time2;
int **list1, *n1, **list2, *n2;
{
  return State_GetVis( log->states, time1, time2, list1, n1, list2, n2 );
}


   /* Message-handling stuff */

int Log_NmsgDefs( log )
logData *log;
{
  return Msg_Ndefs( log->msgs );
}

int Log_GetMsgDef( log, def_num, tag, name, color )
logData *log;
int def_num, *tag;
char **name, **color;
{
  return Msg_GetDef( log->msgs, def_num, tag, name, color );
}

int Log_SetMsgDef( log, def_num, tag, name, color )
logData *log;
int def_num, tag;
char *name, *color;
{
  return Msg_SetDef( log->msgs, def_num, tag, name, color );
}

int Log_Nmsgs( log )
logData *log;
{
  return Msg_N( log->msgs );
}

int Log_GetMsg( log, msg_num, type, sender, recver, size,
		     sendTime, recvTime )
logData *log;
int msg_num;
int *type, *sender, *recver, *size;
double *sendTime, *recvTime;
{
  return Msg_Get( log->msgs, msg_num, type, sender, recver, size,
		  sendTime, recvTime );
}

