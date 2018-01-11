#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#define TESTING 0


/* Sorry about this kludge, but our ANSI C compiler on our suns has broken
   header files */
#if defined(sparc) && defined(__STDC__)
int sscanf( char *, const char *, ... );
#endif


#include "events.h"
#include "states.h"
#include "msgs.h"

#define ALOG_INTERNAL
#include "alog.h"
#include "str_dup.h"


/*
#include "tcl.h"
#include "lists.h"
#include "upshot.h"
*/

#define DEBUG 0

#define MAX_LINE_LEN 1024


/* The format:
Each line:
  type process task data cycle timestamp [comment]

    type - nonnegative integer representing a user-defined event type
    process - an integer representing the process in which the event occurred
    task - an integer representing a different notion of task.  Usually 
           ignored.
    data - an integer representing user data for the event
    cycle - an integer representing a time cycle, used to distinguish
            between time returned by a timer that "rolls over" during
            the run
    timestamp - an integer representing (when considered in conjuction
                with the cycle number) a time for the event.  Upshot treats
                the units as microseconds
    comment - an optional character string representing user data.  Currently
              12 character maximum, might increase to 32 soon.  Programs
              that read the logfile should gracefully handle any
              length, however.

All events from -100 to -1 are reserved header information events.  When
a log is produced, all [-100,-1] events will be moved to the top of the
logfile and have their timestamps set to 0.

All event from -101 and below are reserved system events.  This is to
provide some standardization for the logfiles, so various interpreting
programs can glean similar data from the same logfile.  All (...,-101]
events will have valid timestamps and will be left in time-sorted
order in the logfile.

Formats for reserved types:

  -1 Creation data
     Comment: Creator and date

  -2 Number of events in the logfile
     Data: number of events

  -3 Number of processors in the run
     Data: number of processes

  -4 Number of tasks used in the run
     Task: number of tasks

  -5 Number of event types used
     Data: number event types

  -6 Start time of the run
     Timestamp: start time

  -7 End time of the run
     Timestamp: end time

  -8 Number of times the timer cycled
     For example, if the timer's units are in microseconds, and it has a
     range of 0 - 2^32, and a run lasts 3 hours (range=4294 seconds, 3 hours=
     10800 seconds), the timer would have cycled at least twice.
     Data: number of timer cycles

  -9 Decription of event types
     Data: event type
     Comment: Description

  -10 printf string for event types
      Data: event type
      Comment: printf string

  -11 Rollover point
      The point at which the timer values 'rollover'
      Timestamp: rollover point

  -13 State definition
      Define a state based on the events that signal the beginning and end
      of the state.  Also, define what to call the state and what color/
      stipple pattern to give it in a graphical visualization tool.
      Task: start event
      Data: end event
      Comment: color:bitmap state name

      example:  -12 0 3 4 0 0 Green:boxes Rhode Island
      An event with type 3 will signify the entrance into a 'Rhode Island'
      state.  An event wil type 4 will signify the exit of the 'Rhode Island'
      state.

      States may be overlapped (enter a 'Rhode Island' state while in a
      'Wisconsin' state while in a 'Nevada' state), and the state name may
      have whitspace in it.

  -14 Message definition
      Define a message based on the tag
      Data: message tag
      Comment: color message name

  -100 Synchronization event
       Sync events are used internally to sychronize timers on the various
       processes.  They do not appear in the logfiles.
  -101 Send message
       Represents the sending of a message
       Data: process ID of the receiving process
       Comment: <message-type tag of message> <size of the message, in bytes>

  -102 Receive message
       Represents the receiving of a message
       Data: process ID of the sending process
       Comment: <message-type tag of message> <size of the message, in bytes>

*/






#ifdef __STDC__
static int GetAlogLine( FILE *inf, alogLineData *lineData );
static int StateDef( stateData *state_data, alogData *alog_data,
		     alogLineData *line_data );
static int AlogStateDef( alogData *alog_data,
		         int startEvent, int endEvent );
static int EventDef( eventData *event_data, alogData *alog_data,
		     int eventNum, char *eventName );
static int AlogMsg( logData *log_data, alogData *alog_data,
                    alogLineData *line_data, int type );
static int IsStateEvent( alogData *alog_data, int eventType,
                         int *stateType, int *isStartEvent );
static int AlogEventNum( logData *log_data, alogData *alog_data, int type );
#else
static int GetAlogLine();
static int StateDef();
static int AlogStateDef();
static int EventDef();
static int AlogMsg();
static int IsStateEvent();
static int AlogEventNum();
#endif




static int GetAlogLine( inf, lineData )
FILE *inf;
alogLineData *lineData;
{
  static char line[MAX_LINE_LEN], comment[MAX_LINE_LEN];

  comment[0] = 0;
  if (!fgets( line, MAX_LINE_LEN, inf )) {
#if DEBUG
    fprintf( stderr, "Read eof in GetAlogLine\n" );
#endif
    return -1;  /* EOF */
  }

  if (sscanf( line, "%d %d %d %d %d %lf %[^\n]",
	      &lineData->type, &lineData->process, &lineData->task,
	      &lineData->data, &lineData->cycle, &lineData->timestamp,
	      comment ) < 6) {
#if DEBUG
    fprintf( stderr, "format error in GetAlogLine\n" );
#endif
    return -2;  /* format error */
  }

    /* convert from microseconds to seconds */
  lineData->timestamp *= .000001;

  strncpy( lineData->comment, comment, ALOG_MAX_COMMENT_LEN );

#if DEBUG>1
  fprintf( stderr, "GetAlogLine returning %d %d %d %d %d %f %s\n",
	   lineData->type, lineData->process, lineData->task,
	   lineData->data, lineData->cycle, lineData->timestamp,
	   lineData->comment );
#endif

  return 0;
}



int AlogPreProcessLog( filename, log_data, alog_data )
char *filename;
logData *log_data;
alogData *alog_data;
{
  /* read the header or do any preprocessing needed and create
     the logData structure */

  FILE *fp;
  alogLineData lineData;
  int readStatus, allDone;

  /* create lists for alog state definitions */
  ListCreate( alog_data->stateDefs.list, alogStateDefInfo, 5 );
  ListCreate( alog_data->stateDefs.startEvents, int, 5 );
  ListCreate( alog_data->stateDefs.endEvents, int, 5 );

  /* create lists for alog event definitions */
  ListCreate( alog_data->eventDefs.list, int, 5 );


    /* initialize the e,s, & m data structures */
  log_data->events = Event_Create();
  log_data->states = State_Create();
  log_data->msgs   = Msg_Create();
  
  fp = fopen( filename, "rt" );
    /* is is assumed that the file can be read */
  alog_data->name = STRDUP( filename );
  alog_data->lineNo = 0;
  allDone = 0;

    /* read all header events */
  while (!allDone &&
	 !(readStatus = GetAlogLine( fp, &lineData ))) {
    alog_data->lineNo++;
    if (lineData.type > ALOG_MAX_HEADER_EVT ||
	lineData.type < ALOG_MIN_HEADER_EVT) {
      allDone = 1;
    } else {
#if DEBUG > 1
      fprintf (stderr, "Event %d\n", lineData.type );
#endif
      switch (lineData.type) {
      case ALOG_NP:
	log_data->np = lineData.data;
	break;
      case ALOG_START_TIME:
	log_data->starttime = lineData.timestamp;
	break;
      case ALOG_END_TIME:
	log_data->endtime = lineData.timestamp;
	break;
      case ALOG_EVT_DEF:
	EventDef( log_data->events, alog_data, lineData.data,
		  lineData.comment );
	break;
      case ALOG_ROLLOVER_PT:
	alog_data->rolloverPt = lineData.timestamp;
	break;
      case ALOG_STATE_DEF:
	StateDef( log_data->states, alog_data, &lineData );
	break;
      }
    }
  }


#if TESTING
  State_PrintDefs( log_data->states );
#endif

  if (readStatus == -2) {
    LogFormatError( alog_data->name, alog_data->lineNo );
    return -1;
  }

  alog_data->leftOverLine = lineData;
  alog_data->leftOver_fp = fp;

#if DEBUG
  fprintf( stderr, "alog_data->leftOver_fp left with %p.\n", (void *)fp );
#endif

  return 0;
}


int AlogProcessLog( log_data, alog_data )
logData *log_data;
alogData *alog_data;
{
  int firstLine, readStatus;
  alogLineData lineData;
  int eventType, stateType, isStartEvent;
  FILE *inf;
/*  int isStartEvent, stateType, eventType; */

  Event_DataInit ( log_data->events, log_data->np );
  State_DataInit ( log_data->states, log_data->np );
  Msg_DataInit   ( log_data->msgs, log_data->np );
  
  firstLine = 1;
    /* get leftover line from the preprocessing phase */
  lineData = alog_data->leftOverLine;

    /* copy stream pointer */
  inf = alog_data->leftOver_fp;

  while ( !Log_Halted( log_data ) &&
	  (firstLine ||
	  !(readStatus = GetAlogLine( inf, &lineData ))) ) {
    firstLine = 0;

#if DEBUG >1
    fprintf( stderr, "line: %d %d %d %d %f %s\n", lineData.type,
	     lineData.process,
	     lineData.task, lineData.data, lineData.timestamp,
	     lineData.comment );
#endif

    if (lineData.type == ALOG_MESG_SEND ||
	lineData.type == ALOG_MESG_RECV ) {
      AlogMsg( log_data, alog_data, &lineData, lineData.type );
    } else {
      if (IsStateEvent( alog_data, lineData.type,
		        &stateType, &isStartEvent )) {

	if (isStartEvent) {
	  State_Start( log_data->states, stateType, lineData.process,
		       lineData.timestamp );
	} else {
	  State_End( log_data->states, stateType, lineData.process,
			      lineData.timestamp );
	}

      } else {
	eventType = AlogEventNum( log_data, alog_data, lineData.type );
	Event_Add( log_data->events, eventType, lineData.process,
		   lineData.timestamp );
      }
    }
  }

  if (Log_Halted( log_data )) {
    Log_CloseData( log_data );
    AlogCloseLog( alog_data );
  } else {
    State_DoneAdding( log_data->states );
    Event_DoneAdding( log_data->events );
    Msg_DoneAdding  ( log_data->msgs );

#if TESTING
  State_PrintAll( log_data->states );
  Msg_PrintAll( log_data->msgs );
#endif

  }

  return 0;
}



int AlogCloseLog( alog_data )
alogData *alog_data;
{
  ListDestroy( alog_data->stateDefs.list, alogStateDefInfo* );
  ListDestroy( alog_data->stateDefs.startEvents, int );
  ListDestroy( alog_data->stateDefs.endEvents, int );
  ListDestroy( alog_data->eventDefs.list, int );
  
  free( alog_data );
  return 0;
}



static int StateDef( state_data, alog_data, lineData )
stateData *state_data;
alogData *alog_data;
alogLineData *lineData;
{
  char *color=0, *bitmap=0, *name=0;
  int stateType, isStartEvent;

  /* break the line_data->comment string up into bitmap:color name */

  if (IsStateEvent( alog_data, lineData->task, &stateType, &isStartEvent )) {
      /* if the state being defined is already defined, skip it */
    return 0;
  }

  bitmap = color = lineData->comment;

  while (*bitmap && *bitmap!=':') bitmap++;
  /* look for separating colon */

  if (*bitmap) {
    *bitmap = '\0';
    name = ++bitmap;
    while (*name && !isspace(*name)) name++;
    /* skip over the bitmap */

    while (*name && isspace(*name)) *name++ = '\0';
    /* fill space between bitmap and name with terminators */

    if (!*name) {
      /* if there is no name */
      LogFormatError( alog_data->name, alog_data->lineNo );
    }
  } else {
    LogFormatError( alog_data->name, alog_data->lineNo );
  }
  if (!(color && bitmap && name)) {
    LogFormatError( alog_data->name, alog_data->lineNo );
  } else {
    State_AddDef( state_data, color, bitmap, name );
    AlogStateDef( alog_data, lineData->task, lineData->data );
  }
  return 0;
}



static int AlogStateDef( alog_data, startEvent, endEvent )
alogData *alog_data;
int startEvent, endEvent;
{
  alogStateDefInfo info;

  info.startEvt = startEvent;
  info.endEvt = endEvent;

  ListAddItem( alog_data->stateDefs.list, alogStateDefInfo, info );
  ListAddItem( alog_data->stateDefs.startEvents, int, startEvent );
  ListAddItem( alog_data->stateDefs.endEvents, int, endEvent );

  return 0;
}


static int IsStateEvent( alog_data, eventType, stateType, isStartEvent )
alogData *alog_data;
int eventType, *stateType, *isStartEvent;
{
  int ndefs, *startEvt, *endEvt, i;

  ndefs = ListSize( alog_data->stateDefs.startEvents, int );
  startEvt = ListHeadPtr( alog_data->stateDefs.startEvents, int );
  endEvt = ListHeadPtr( alog_data->stateDefs.endEvents, int );

  for (i=0; i<ndefs; i++,startEvt++,endEvt++) {
    if (*startEvt == eventType) {
      *stateType = i;
      *isStartEvent = 1;
      return 1;
    } else if (*endEvt == eventType) {
      *stateType = i;
      *isStartEvent = 0;
      return 1;
    }
  }

  return 0;
}


static int EventDef( event_data, alog_data, eventNum, eventName )
eventData *event_data;
alogData *alog_data;
int eventNum;
char *eventName;
{
  Event_AddDef( event_data, eventName );
  ListAddItem( alog_data->eventDefs.list, int, eventNum );
  return 0;
}


static int AlogEventNum( log_data, alog_data, event )
logData *log_data;
alogData *alog_data;
int event;
{
  int nevents, i, *ptr;

  ptr = ListHeadPtr( alog_data->eventDefs.list, int );
  nevents = ListSize( alog_data->eventDefs.list, int );

  /* look for event definition */
  for (i=0; i<nevents; i++,ptr++) {
    if (*ptr == event) {
      return i;
    }
  }

  /* if not found, create new one */
  EventDef( log_data->events, alog_data, event, "" );

  return i;
}


static int AlogMsg( log_data, alog_data, line_data, type )
logData *log_data;
alogData *alog_data;
alogLineData *line_data;
int type;
{
  int tag, size;

  if (sscanf( line_data->comment, "%d %d", &tag, &size ) != 2) {
    LogFormatError( alog_data->name, alog_data->lineNo );
    return -1;
  }

  if (type == ALOG_MESG_SEND) {
    Msg_Send( log_data->msgs, line_data->process, line_data->data,
	      line_data->timestamp, tag, size );
  } else {
    Msg_Recv( log_data->msgs, line_data->process, line_data->data,
	      line_data->timestamp, tag, size );
  }

  return 0;
}
