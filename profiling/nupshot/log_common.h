/*
   Logfile stuff common to all formats for Upshot.

   Ed Karrels
   Argonne National Laboratory
*/

#ifndef _LOG_COMMON_H_
#define _LOG_COMMON_H_


#include "events.h"
#include "states.h"
#include "msgs.h"
/*
#include "pctdone.h"
*/

typedef enum logFormat_ {
  unknown_format,
  alog_format,
  blog_format,
  old_picl_format,
  new_picl_format,
  sddf_format,
  vt_format
} logFormat;

typedef struct logData_ {
  stateData *states;		/* info about states */
  eventData *events;		/* info about standalone events */
  msgData *msgs;		/* info about messages */
  double starttime, endtime;	/* start and end time of the logfile */
  int np;			/* number of processors */
/*  pctDone *pct_done; */
  int loaded;			/* whether the logfile has been loaded yet */
} logData;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif


int LogFormatError ARGS(( char *filename, int line ));
  /* Tell the user about a log format error--possibly the wrong
     log format. */

double Log_StartTime ARGS(( logData * ));
double Log_EndTime ARGS(( logData * ));
int Log_Np ARGS(( logData * ));
int Log_Loaded ARGS(( logData * ));

#endif
