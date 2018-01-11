/*
   Format-unspecific logfile stuff for Upshot.

   Ed Karrels
   Argonne National Laboratory
*/

#ifndef _LOG_H_
#define _LOG_H_

#include "alog.h"


typedef struct commonLogData_ {
  logFormat format;		/* logfile format */
  char *name;			/* filename of the logfile */
  int lineNo;			/* current line */
} commonLogData;

typedef union anyLogData_ {
  logFormat format;
  commonLogData common;
  alogData alog;
  /* oldPiclData oldPicl; */
} logFile;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif


logFile *Log_OpenFile ARGS(( char *format ));

int PreProcessLog ARGS(( char *filename,
		         logData *log_data, logFile *log_file ));
  /* call the appropriate processor to open the logfile */

int ProcessLog ARGS(( logData *log_data, logFile *log_file ));
  /* call the appropriate processor to load the logfile */

int Log_Close ARGS(( logData *log_data, logFile *log_file ));
  /* close the logfile, free memory, etc. */

int LogFormatError ARGS(( char *filename, int line ));
  /* Tell the user about a log format error--possibly the wrong
     log format. */

int Log_NeventDefs    ARGS(( logData *log ));
int Log_GetEventDef   ARGS(( logData *log, int def_num, char **name ));
int Log_SetEventDef   ARGS(( logData *log, int def_num, char *name ));
int Log_Nevents       ARGS(( logData *log ));
int Log_GetEvent      ARGS(( logData *log, int n, int *type,
			     int *proc, double *time ));

int Log_NstateDefs    ARGS(( logData *log ));
int Log_GetStateDef   ARGS(( logData *log, int def_num, char **name,
			     char **color, char **bitmap ));
int Log_SetStateDef   ARGS(( logData *log, int def_num, char *name,
			     char *color, char *bitmap ));
int Log_Nstates       ARGS(( logData *log ));
int Log_GetState      ARGS(( logData *log, int n, int *type, int *proc,
			     double *startTime, double *endTime,
			     int *parent, int *firstChild,
			     int *overlapLevel ));
void *Log_AddDrawState  ARGS(( logData *log, drawStateFn *fn, void *data ));
int Log_RmDrawState   ARGS(( logData *log, void *token ));
int Log_GetVisStates  ARGS(( logData *log, double from_time, double to_time,
			     int **list1, int *n1, int **list2, int *n2 ));

int Log_NmsgDefs      ARGS(( logData *log ));
int Log_GetMsgDef     ARGS(( logData *log, int n, int *tag, char **name,
			     char **color ));
int Log_SetMsgDef     ARGS(( logData *log, int n, int tag, char *name,
			     char *color ));
int Log_Nmsgs         ARGS(( logData *log ));
int Log_GetMsg        ARGS(( logData *log, int n, int *type, int *sender,
			     int *recver, int *size, double *sendTime,
			     double *recvTime ));


#endif

