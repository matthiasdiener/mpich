/*
   Tcl wrappers for unsuspecting logfile routines

   Ed Karrels
   Argonne National Laboratory
*/


#define DEBUG 0

/*

These functions provide an object-like Tcl interface to logfiles,
while providing efficient C hooks for real number crunching.

Command list:

  logfile open <filename> <format>
    Opens the named logfile as a logfile of the given format.
    Formats recognized: alog
    returns: identifying token for this logfile used by all other
             logfile commands

  logfile <token> load [options]
    Load the logfile previously opened.  No options are supported yet.

  logfile <token> write <filename> <format>
    Write out a logfile to the specified file and format.
    Formats recognized: alog

  logfile <token> close
    Close a logfile previously opened and possible loaded.
    The token may not be used after the logfile is closed.

  logfile <token> np
    Returns the number of processes in the run described by the logfile.

  logfile <token> starttime
    Returns the start time in the logfile.

  logfile <token> endtime
    Returns the end time in the logfile.

  logfile <token> neventdefs
    Returns the number of event description

  logfile <token> eventdef <idx>
    Returns the description of event type idx:
       {name}
        name - name of the event

  logfile <token> set eventdef <idx> <name>
    Change the description of an event.

  logfile <token> nstatedefs
    Returns the number of state descriptions

  logfile <token> statedef <idx>
    Returns the description of state type idx:
       {name color bitmap}
        name - name of the state
        color - color with which to draw the state
        bitmap - when in B&W, stipple bitmap with which to draw the state

  logfile <token> set statedef <idx> <name> <color> <bitmap>
    Change the description of a state.

  logfile <token> nmsgdefs
    Returns the number of message descriptions

  logfile <token> msgdef <idx>
    Returns the description of message type idx:
       {tag color}
        tag - integer tag of the message
        color - color with which to draw the message

  logfile <token> set msgdef <idx> <tag> <color>
    Change the description of a message

  logfile <token> nevents
    Returns the number of standalone events described in the logfile.
    Only available after the logfile is loaded.

  logfile <token> event <idx>
    Returns event number idx in the logfile as a list:
       {proc type time}
        proc - the process on which the event occurred
        type - index of the event type that describes this instance
        time - the time the event occurred
    Only available after the logfile is loaded.

  logfile <token> idx events all
    Returns list of integer indices of all events sorted by time

  logfile <token> idx events proc <proc>
    Returns list of integer indices of all events that occurred
    on the given process sorted by time

  logfile <token> nstates
    Returns the number of states described in the logfile.  Only available
    after the logfile is loaded.

### -more indices for states- ###
### leave out indices for now ###

  logfile <token> state <idx>
    Returns state number idx in the logfile as a list:
       {proc type startTime endTime within}
        proc - the process on which the state occurred
        type - index of the state type that describes this instance
        startTime, endTime - start and end times of the state
        within - index of the state inside which this state started;
                 -1 if none
    Only available after the logfile is loaded.

  logfile <token> nmsgs
    Returns the number of messages described in the logfile.  Only available
    after the logfile is loaded.

  logfile <token> msg <idx>
    Returns message number idx in the logfile as a list:
       {sender receiver tag size sendTime recvTime}
        sender - sending process
        receiver - receiving process
        tag - integer tag of message
        size - size of the message
        sendTime - time the message was sent
        recvTime - time the message was received
    Only available after the logfile is loaded.

  logfile <token> idx msg sendtime
    Returns a list the indices of all the messages in the logfile sorted in
    order the time sent.
    Only available after the logfile is loaded.

  logfile <token> idx msg recvtime
    Returns a list the indices of all the messages in the logfile sorted in
    order the time received.
    Only available after the logfile is loaded.

  logfile <token> adjust offset <proc> <offset>
    Adjust all the events, states, and messages for a given process
    by a given offset.  Used for adjusting logs gathered on machines
    without syncronized clocks.

  logfile <token> adjust skew <proc> <origin> <skew>
    Adjust all the events, states, and messages for a given process
    by a given skew factor from a given origin.  For example, if a
    process logged events at times of 25 and 100, and a skew adjustment
    is made with 50 as the origin and a factor of 2, the event will
    be moved to times of 0 and 150, respectively.  Used for adjusting logs
    gathered on machines without syncronized clocks.

*/


#include "tcl.h"
#include "tk.h"
#include "tclptr.h"
#include "log.h"
#include "log_widget.h"

/* Sorry about this kludge, but our ANSI C compiler on our suns has broken
   header files */
#if defined(sparc) && defined(__STDC__)
int fclose( FILE * );
#endif


typedef enum widgCmd {
  NONE = -1,
  LOAD,
  WRITE,
  CLOSE,
  NP,
  STARTTIME,
  ENDTIME,
  NEVENTDEFS, 
  NSTATEDEFS,
  NMSGDEFS,
  NEVENTS,
  NSTATES,
  NMSGS,
  GET_EVENTDEF,
  SET_EVENTDEF,
  GET_STATEDEF,
  SET_STATEDEF,
  GET_MSGDEF,
  SET_MSGDEF,
  GET_EVENT,
  GET_STATE,
  GET_MSG,
  ADJUST_OFFSET,
  ADJUST_SKEW
} widgCmd;


#ifdef __STDC__
static int LogOpen( Tcl_Interp*, int argc, char *argv[] );
#define HELPER_ARGS ( Tcl_Interp *interp, logWidget *log, \
		      int argc, char *argv[] )
static widgCmd GetCommand( Tcl_Interp*, int argc, char *argv[] );
static int SimpleInfo ( Tcl_Interp *interp, logWidget *log, \
		        int argc, char *argv[], widgCmd command );
#else
static int LogOpen();
#define HELPER_ARGS ();
typedef int widget_helper();
static widgCmd GetCommand();
static int SimpleInfo();
#endif

static int LogLoad HELPER_ARGS;
static int LogWrite HELPER_ARGS;
static int LogClose HELPER_ARGS;
static int Get_EventDef HELPER_ARGS;
static int Set_EventDef HELPER_ARGS;
static int Get_StateDef HELPER_ARGS;
static int Set_StateDef HELPER_ARGS;
static int Get_MsgDef HELPER_ARGS;
static int Set_MsgDef HELPER_ARGS;
static int Get_Event HELPER_ARGS;
static int Get_State HELPER_ARGS;
static int Get_Msg HELPER_ARGS;
static int Offset HELPER_ARGS;
static int Skew HELPER_ARGS;

int logfileAppInit( interp )
Tcl_Interp *interp;
{
  Tcl_CreateCommand( interp, "logfile", LogFileCmd, (ClientData) 0,
		     (Tcl_CmdDeleteProc*) 0 );
  return 0;
}


/* convert string of the form "log%d" to logDataAndFile * */
logWidget *LogToken2Ptr( interp, str )
Tcl_Interp *interp;
char *str;
{
  logWidget *log=0;

  if (strncmp( str, "log", 3 ) ||
      !(log = GetTclPtr( atoi( str+3 ) ))) {
    Tcl_AppendResult( interp, "\"", str,
		      "\"--unrecognized logfile token.", (char *)0);
  }
  return log;
}


int LogFileCmd( clientData, interp, argc, argv )
ClientData clientData;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  logWidget *log;
  widgCmd command;

  if (argc >= 2) {
    if (!strcmp( argv[1], "open" )) {
      return LogOpen( interp, argc, argv );
    } else {

      if (!(log = LogToken2Ptr( interp, argv[1] ))) {
	return TCL_ERROR;
      }

        /* Parse the remaining arguments to figure out what the
	   command is.  On error, set interp->result and return NONE */
      command = GetCommand( interp, argc-2, argv+2 );
      
      if (command == NONE) {
	return TCL_ERROR;
      }
      
      switch (command) {
      case LOAD:
	return LogLoad( interp, log, argc, argv );
      case WRITE:
	return LogWrite( interp, log, argc, argv );
      case CLOSE:
	return LogClose( interp, log, argc, argv );
      case NP:
      case STARTTIME:
      case ENDTIME:
      case NEVENTDEFS:
      case NSTATEDEFS:
      case NMSGDEFS:
      case NEVENTS:
      case NSTATES:
      case NMSGS:
	return SimpleInfo( interp, log, argc, argv, command );
      case SET_EVENTDEF:
	return Set_EventDef( interp, log, argc, argv );
      case GET_EVENTDEF:
	return Get_EventDef( interp, log, argc, argv );
      case SET_STATEDEF:
	return Set_StateDef( interp, log, argc, argv );
      case GET_STATEDEF:
	return Get_StateDef( interp, log, argc, argv );
      case SET_MSGDEF:
	return Set_MsgDef( interp, log, argc, argv );
      case GET_MSGDEF:
	return Get_MsgDef( interp, log, argc, argv );
      case GET_EVENT:
	return Get_Event( interp, log, argc, argv );
      case GET_STATE:
	return Get_State( interp, log, argc, argv );
      case GET_MSG:
	return Get_Msg( interp, log, argc, argv );
      case ADJUST_OFFSET:
	return Offset( interp, log, argc, argv );
      case ADJUST_SKEW:
	return Skew( interp, log, argc, argv );
      default:
	Tcl_AppendResult( interp, "\"", argv[2], "\"--unrecognized command.",
			  (char *)0 );
	return TCL_ERROR;
      }
    }
  }
    
  Tcl_AppendResult( interp, argv[0], " needs a command.", (char *)0 );
  return TCL_ERROR;

}

static struct {
  char *str;
  widgCmd cmd;
} single_word_cmds[] = {
  {"load", LOAD},
  {"write", WRITE},
  {"close", CLOSE},
  {"np", NP},
  {"starttime", STARTTIME},
  {"endtime", ENDTIME},

  {"neventdefs", NEVENTDEFS},
  {"get_eventdef", GET_EVENTDEF},
  {"set_eventdef", SET_EVENTDEF},
  {"nevents", NEVENTS},
  {"get_event", GET_EVENT},

  {"nstatedefs", NSTATEDEFS},
  {"get_statedef", GET_STATEDEF},
  {"set_statedef", SET_STATEDEF},
  {"nstates", NSTATES},
  {"get_state", GET_STATE},

  {"nmsgdefs", NMSGDEFS},
  {"get_msgdef", GET_MSGDEF},
  {"set_msgdef", SET_MSGDEF},
  {"nmsgs", NMSGS},
  {"get_msg", GET_MSG}};


static widgCmd GetCommand( interp, argc, argv )
Tcl_Interp *interp;
int argc;	      /* note that the first two arguments have been */
char *argv[];         /* trimmed before being passed to this functon */
{
  int ncmds, i;

  ncmds = sizeof single_word_cmds / sizeof single_word_cmds[0];

  for (i=0; i<ncmds; i++) {
#if DEBUG
    fprintf( stderr, "comparing %s and %s\n", argv[0],
	     single_word_cmds[i].str );
#endif
    if (!strcmp(argv[0], single_word_cmds[i].str)) {
      return single_word_cmds[i].cmd;
    }
  }

  Tcl_AppendResult( interp, argv[0], ": Unrecognized command for logfile.  ",
		    "Must be one ",
		    "of: load, write, close, np, starttime, endtime, ",
		    "neventdefs, get_eventdef, set_eventdef, nevents, ",
		    "get_event, nstatedefs, get_statedef, set_statedef, ",
		    "nstates, get_state, nmsgdefs, get_msgdef, set_msgdef, ",
		    "nmsgs, get_msg.", (char*)0 );

  return NONE;
}


static int LogOpen( interp, argc, argv )
Tcl_Interp *interp;
int argc;
char *argv[];
{
  FILE *fp;
  logWidget *log;
  int ptrIdx;
  char ptrStr[30];

  if (argc < 4) {
    Tcl_AppendResult( interp, "Bad args to ", argv[0], " ",
		      argv[1], ": <filename> <format>", (char *)0 );
    return TCL_ERROR;
  }

  /* take care of extra args - tk configuration help? */

    /* make sure the file can be opened */
  if ((fp = fopen( argv[2], "r" ))) {
    fclose( fp );
  } else {
    Tcl_AppendResult( interp, "Couldn't open ", argv[2], (char *)0 );
    return TCL_ERROR;
  }

    /* allocate space for log data structure */
  if (!(log = (logWidget*) malloc( sizeof( logWidget ) )) ||
      !(log->data = Log_OpenData()) ||
      !(log->file = Log_OpenFile( argv[3] ))) {
    Tcl_SetResult( interp, "Out of memory", TCL_STATIC );
    return TCL_ERROR;
  }

  ptrIdx = AllocTclPtr( (void *)log );
  sprintf( ptrStr, "log%d", ptrIdx );

  if (-1 == PreProcessLog( argv[2], log->data, log->file )) {
      /* count on PreProcessLog to free its stuff if it dies */
    Log_Close( log->data, log->file );
    free( (void*)log );
    Tcl_AppendResult( interp, argv[3], ": unknown format", (char*)0 );
    return TCL_ERROR;
  } else {
      /* return string in th form log%d, %d being an index into
         the tclptr array */
    Tcl_SetResult( interp, ptrStr, TCL_VOLATILE );
    return TCL_OK;
  }
}


static int LogWrite( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}


static int LogClose( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  switch (Log_Close( log->data, log->file )) {
  case 0:
    return TCL_OK;
  default:
    Tcl_AppendResult( interp, "Error closing logfile.", (char *)0 );
    return TCL_ERROR;
  }
}



static int LogLoad( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  switch (ProcessLog( log->data, log->file )) {
  case 0:
    return TCL_OK;
  default:
    Tcl_AppendResult( interp, "Error loading logfile.", (char *)0 );
    return TCL_ERROR;
  }
}


static int SimpleInfo( interp, log, argc, argv, command )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
widgCmd command;
{
  char buf[100];		/* buffer for holding the result */
  int num;			/* copy of value for integer results */
  double time;			/* copy of value for double results */

  if (command==NEVENTS || command==NSTATES || command==NMSGS) {
    /* make sure the logfile has been loaded */
    if (!Log_Loaded( log->data )) {
      Tcl_SetResult( interp, "logfile has not yet been loaded",
		     TCL_STATIC );
      return TCL_ERROR;
    }
  }

  /* get the integer or double we want, set to 0 if the command is not
     recognized--shouldnt happen */
  if (command == STARTTIME || command == ENDTIME) {
    time = (command == STARTTIME) ? Log_StartTime( log->data )
                                  : Log_EndTime( log->data );
    sprintf( buf, "%.6f", time );
  } else {
    num = (command == NP) ?
            Log_Np( log->data ) :
	  (command == NEVENTDEFS) ?
	    Log_NeventDefs( log->data ) :
          (command == NSTATEDEFS) ?
	    Log_NstateDefs( log->data ) :
          (command == NMSGDEFS) ?
	    Log_NmsgDefs( log->data ) :
	  (command == NEVENTS) ?
	    Log_Nevents( log->data ) :
	  (command == NSTATES) ?
	    Log_Nstates( log->data ) :
          (command == NMSGS) ?
	    Log_Nmsgs( log->data ) : -1;
    sprintf( buf, "%d", num );
  }

  Tcl_SetResult( interp, buf, TCL_VOLATILE );
  return TCL_OK;
}


static int Get_EventDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Set_EventDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

  /* return a state definition as a list of <name> <color> <bitmap> */
static int Get_StateDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  int i;
  char *name, *color, *bitmap;

    /* check the number of arguments */
  if (argc != 4) {
    Tcl_SetResult( interp, "Wrong # of args to logfile statedef",
		  TCL_STATIC );
    return TCL_ERROR;
  }

    /* convert the index given into a string and check the range */
  i = atoi( argv[3] );
  if (i<0 || i>=Log_NstateDefs( log->data )) {
    Tcl_SetResult( interp, "out of range index to logfile statedef",
		  TCL_STATIC );
    return TCL_ERROR;
  }

  Log_GetStateDef( log->data, i, &name, &color, &bitmap );
  Tcl_AppendElement( interp, name );
  Tcl_AppendElement( interp, color );
  Tcl_AppendElement( interp, bitmap );

  return TCL_OK;
}

static int Set_StateDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Get_MsgDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Set_MsgDef( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Get_Event( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Get_State( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  int i;
  int type, proc, parent, firstChild, overlapLevel;
  double startTime, endTime;

  if (argc != 4) {
    Tcl_SetResult( interp, "Wrong # of args to logfile state",
		  TCL_STATIC );
    return TCL_ERROR;
  }

  i = atoi( argv[3] );
  if (i<0 || i>=Log_Nstates( log->data )) {
    Tcl_SetResult( interp, "out of range index to logfile state",
		  TCL_STATIC );
    return TCL_ERROR;
  }

  Log_GetState( log->data, i, &type, &proc, &startTime, &endTime,
	        &parent, &firstChild, &overlapLevel );

  sprintf( interp->result, "%d %d %.17g %.17g %d %d %d",
	   type, proc, startTime, endTime, parent, firstChild, overlapLevel );

  return TCL_OK;
}

static int Get_Msg( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Offset( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

static int Skew( interp, log, argc, argv )
Tcl_Interp *interp;
logWidget *log;
int argc;
char *argv[];
{
  return TCL_OK;
}

