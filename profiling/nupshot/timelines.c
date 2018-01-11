#if !defined(HAVE_STDLIB_H)
#include <stdlib.h>
#else
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#endif
#include "tcl.h"
#include "tk.h"
#include "log.h"
#include "log_widget.h"
#include "timelines.h"
#include "tclptr.h"
#include "str_dup.h"
#include "tcl_callargv.h"
#include "cvt_args.h"


/* Sorry about this kludge, but our ANSI C compiler on our suns has broken
   header files */
#if defined(sparc) && defined(__STDC__)
int sscanf( char *, const char *, ... );
#endif

#define DEBUG 0
#define DEBUG_ZOOM 0
#define DEBUG_VIS 0
#define DEBUG_TK_DRAWS 0
#define DEBUG_UPDATE 0
#define DEBUG_UPDATE_GOOFINESS 0
#define DEBUG_NO_CONFIG 0
#define DEBUG_EVENTS 0
#define DEBUG_DESTROY 0
#define DEBUG_RESIZE 1

  /* length to use for temporary Tcl commands */
#define TMP_CMD_LEN 200

  /* macro for calling a Tcl function manually, checking the return,
     and printing an error message on failure.  No return, no exit(). */


#define Pix2Time( tl, pix )  ((pix)  * (tl)->pix2time + (tl)->starttime)
#define Time2Pix( tl, time ) (((time) - (tl)->starttime) / (tl)->pix2time)
#define Pix2Proc( tl, pix )  ((pix)  * (tl)->pix2proc)
#define Proc2Pix( tl, proc ) ((proc) / (tl)->pix2proc)



#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

/*
   The function that is called by the tcl routine "timeline".
   Widget creation command.  clientData is (Tk_Window)mainWin.
*/
static int Timeline_Init ARGS((ClientData  clientData, Tcl_Interp *interp,
			   int argc, char *argv[]));

/*
   Link all the variable in the tl structure to Tcl variables.
*/
static int LinkVars ARGS(( Tcl_Interp *interp, timeLineInfo *tl,
			   char *array_name, char *idx_prefix ));


static int SetOverlap ARGS(( xpandList /*int*/ *list2 ));
static int StateOnScreen ARGS(( timeLineInfo *tl, int type, int proc ));

/*
   Draw one state.
   The void *data is a timeLineInfo*.
*/
static int TimeLineDrawState ARGS(( void *data, int idx, int type, int proc,
				    double startTime, double endTime,
				    int parent, int firstChild,
				    int overlapLevel ));

/*
   Get timeLineInfo* from a window name.  $timeline($win,tl)
*/
static timeLineInfo *TlPtrFromWindow ARGS(( Tcl_Interp *interp,
					    char *win ));

#if 0
/*
   The function that is called by Tcl scripts for all the timeline
   widget commands.
*/
static int Timeline_Cmd ARGS(( ClientData, Tcl_Interp *, int argc,
			      char **argv ));
#endif

/*
   Return descripion of current item.
*/
static int CurrentItem ARGS(( ClientData, Tcl_Interp *, int argc,
			       char **argv ));

/*
   Passed to Tk_EventuallyFree to close the widget record.
*/
static int Destroy ARGS(( ClientData, Tcl_Interp*, int, char** ));


#if DEBUG
static int PrintStates ARGS(( timeLineInfo* ));
#endif



#define UPDATES_EVERY_N_STATES 200



static int SegFaultCmd( clientData, interp, argc, argv )
ClientData clientData;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  strcpy( (char*)0, "My baloney has a first name, it's O-S-C-A-R." );
  return 0;
}



int timelineAppInit( interp )
Tcl_Interp *interp;
{
  Tcl_CreateCommand( interp, "Timeline_C_Init", Timeline_Init,
		     (ClientData)0, (Tcl_CmdDeleteProc*) 0 );

/*
  Tcl_CreateCommand( interp, "Timeline_C_Cmd", Timeline_Cmd,
		     (ClientData)0, (Tcl_CmdDeleteProc*) 0 );
*/

  Tcl_CreateCommand( interp, "Timeline_CurrentItem", CurrentItem,
		     (ClientData)0, (Tcl_CmdDeleteProc*) 0 );

  Tcl_CreateCommand( interp, "Timeline_C_Destroy", Destroy,
		     (ClientData)0, (Tcl_CmdDeleteProc*) 0 );

  Tcl_CreateCommand( interp, "SegFault", SegFaultCmd,
		     (ClientData)0, (Tcl_CmdDeleteProc*) 0 );

  return 0;
}




static int Timeline_Init( clientData, interp, argc, argv )
ClientData clientData;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  int i, n;
  logWidget *log;
  timeLineInfo *tl;
  char *logToken, *windowName, *varName;

    /* convert the arguments from strings, put them in the widget record */
  if (TCL_OK != ConvertArgs( interp, "timeline <window> <logfile token>", 
		   "1 ss", argc, argv, &windowName, &logToken )) {
    goto failed_1;
  }

  tl = (timeLineInfo*) malloc( sizeof(timeLineInfo) );
  if (!tl) {
    Tcl_AppendResult( interp, "Failed to allocate memory for timeline ",
		      "widget record.", (char *)0 );
    return TCL_ERROR;
  }

    /* grab pointer to interpreter */
  tl->interp = interp;

    /* save copy of my window name */
  tl->windowName = STRDUP( windowName );

    /* build canvas name, save copy */
  tl->canvasName = (char *)malloc( strlen(tl->windowName)+3 );
  sprintf( tl->canvasName, "%s.c", tl->windowName );

    /* convert the logfile token to a string, get the logData* */
  if (!(log = LogToken2Ptr( interp, logToken ))) {
      /* LogToken2Ptr sets interp->result */
    goto failed_2;
  }

    /* keep pointer to logfile data structure */
  tl->log = log->data;

  tl->starttime = Log_StartTime( tl->log );
  tl->endtime = Log_EndTime( tl->log );

    /* create visiblity definitions such that everything is visible */
    /* might want to declare a special case for the visible object for
       the sake of speed in which everything is visible, no checking
       needed */
  tl->procVis = Vis_new();
  for (i=0, n=Log_Np( tl->log ); i < n; i++) {
    Vis_add( tl->procVis, i, -1 );
  }

  tl->eventVis = Vis_new();
  for (i=0, n=Log_NeventDefs( tl->log ); i < n; i++) {
    Vis_add( tl->eventVis, i, -1 );
  }

  tl->stateVis = Vis_new();
  for (i=0, n=Log_NstateDefs( tl->log ); i < n; i++) {
    Vis_add( tl->stateVis, i, -1 );
  }

  tl->msgVis = Vis_new();
  for (i=0, n=Log_NmsgDefs( tl->log ); i < n; i++) {
    Vis_add( tl->msgVis, i, -1 );
  }

    /* get the list of overlap levels, you know, how wide to draw
       a state bar if is is overlapping X other states */
  SetOverlap( &tl->overlap.halfWidths );

    /* create list for for state drawing information */
    /* don't forget we still need to add messages and events */
  ListCreate( tl->stateList, tl_stateInfo, 100 );

    /* add this timeline display to the list of function to be called
       whenever a new state is read */
  tl->drawStateToken = 
    Log_AddDrawState( tl->log, TimeLineDrawState, (void*)tl );

    /* create variables like timeline(.f.1.ge.sdf,fg) */
  varName = (char*) malloc( strlen(windowName) + 11 + 20 );

  tl->bg = 0;
  tl->outlineColor = 0;
  tl->pix2time = tl->pix2proc = 1;
  tl->bw = 42;

    /* allow Tcl to control height, width, fg, bg, the scrollbar-like setting,
       starttime, endtime, and the pix2time, pix2proc conversion factors */
  LinkVars( interp, tl, "timeline", tl->windowName );

  sprintf( interp->result, "%d", AllocTclPtr( (void*)tl ) );

    /* like, wow, no errors! */
  return TCL_OK;

 failed_2:
  free( tl->windowName );
  free( tl->canvasName );
  free( tl );
 failed_1:
  return TCL_ERROR;
}



#define LINK_ELEMENT( str_name, name, type ) \
  sprintf( tmp, "%s(%s,%s)", array_name, idx_prefix, str_name ); \
  Tcl_LinkVar( interp, tmp, (char*)&tl->name, type );
/*
  fprintf( stderr, "Linking tl->%s at address %p.\n", \
	   str_name, (char*)&tl->name );
*/


static int LinkVars( interp, tl, array_name, idx_prefix )
Tcl_Interp *interp;
timeLineInfo *tl;
char *array_name;
char *idx_prefix;
{
  char *tmp;
  int i = TCL_LINK_INT;
  int f = TCL_LINK_DOUBLE;
  int s = TCL_LINK_STRING;

  /* tmp will get filled with something like "timeline(.win.0,stuff)",
     or ("%s(%s,%s)", array_name, idx_prefix, element_name ), to
     be exact.
  */

  int max_element_len = 12;  /* "windowUnits" */

  tmp = malloc( strlen(array_name) + 1 + strlen(idx_prefix) + 1 +
	        max_element_len + 1 + 1 );
  /*
    width i
    height i
    bg s
    outlineColor s
    totalUnits i
    windowUnits i
    firstUnit i
    lastUnit i
    pix2time f
    pix2proc f
    bw i
    starttime f
    endtime f
  */

  LINK_ELEMENT( "width", width, f );
  LINK_ELEMENT( "height", height, f );
  LINK_ELEMENT( "bg", bg, s );
  LINK_ELEMENT( "outlineColor", outlineColor, s );
  LINK_ELEMENT( "totalUnits", totalUnits, i );
  LINK_ELEMENT( "windowUnits", windowUnits, i );
  LINK_ELEMENT( "firstUnit", firstUnit, i );
  LINK_ELEMENT( "lastUnit", lastUnit, i );
  LINK_ELEMENT( "pix2time", pix2time, f );
  LINK_ELEMENT( "pix2proc", pix2proc, f );
  LINK_ELEMENT( "bw", bw, i );
  LINK_ELEMENT( "starttime", starttime, f );
  LINK_ELEMENT( "endtime", endtime, f );

  free( tmp );

  return 0;
}



static timeLineInfo *TlPtrFromToken( interp, token )
Tcl_Interp *interp;
char *token;
{
  timeLineInfo *tl;

  tl = (timeLineInfo*) GetTclPtr( atoi( token ) );
  if (tl) {
    return tl;
  } else {
    Tcl_AppendResult( interp, token, " -- invalid timeline token",
		     (char *)0 );
    return 0;
  }
}


static timeLineInfo *TlPtrFromWindow( interp, win )
Tcl_Interp *interp;
char *win;
{
  char *tmp;
  char *tl_str;

  tmp = (char*)malloc( strlen(win) + 4 );
  sprintf( tmp, "%s,tl", win );
  tl_str = Tcl_GetVar2( interp, "timeline", tmp, TCL_GLOBAL_ONLY );
  free( tmp );

  if (tl_str) {
    return TlPtrFromToken( interp, tl_str );
  } else {
    Tcl_AppendResult( interp, "timeline(", win, ",tl) doesn't exist.",
		      (char *)0 );
    return 0;
  }
}



static int Destroy( data, interp, argc, argv )
ClientData data;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  timeLineInfo *tl;
  char *varName;

  if (argc != 2) {
    Tcl_AppendResult( interp, "wrong # of args: ", argv[0], " <token>",
		      (char*)0 );
    return TCL_ERROR;
  }

    /* convert data pointer to my widget record pointer */
  tl = TlPtrFromToken( interp, argv[1] );
  if (!tl) {
    return TCL_ERROR;
  }

  ListDestroy( tl->stateList, tl_stateInfo );

    /* create variables like timeline(.f.1.ge.sdf,fg) */
  varName = (char*) malloc( strlen(tl->windowName) + 11 + 20 );


  sprintf( varName, "timeline(%s,width)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,height)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,bg)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );
  if (tl->bg) {
    free(tl->bg);
    tl->bg = 0;
  }

  sprintf( varName, "timeline(%s,outlineColor)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );
  if (tl->outlineColor) {
    free(tl->outlineColor);
    tl->outlineColor = 0;
  }

  sprintf( varName, "timeline(%s,totalUnits)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,windowUnits)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,firstUnit)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,lastUnit)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,pix2time)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,pix2proc)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,bw)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,starttime)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  sprintf( varName, "timeline(%s,endtime)", tl->windowName );
  Tcl_UnlinkVar( interp, varName );

  free( varName );

  Vis_close( tl->procVis );
  Vis_close( tl->eventVis );
  Vis_close( tl->stateVis );
  Vis_close( tl->msgVis );
  tl->procVis = tl->eventVis = tl->stateVis = tl->msgVis = 0;

  free( tl->windowName );	/* STRDUP'd */
  free( tl->canvasName );	/* malloc()'d */
  tl->windowName = tl->canvasName = 0;

  free( (void*)tl );			/* malloc()'d */

  return TCL_OK;
}



/*
   Overlap levels are a percentage - 0 to 100, how much of the timeline's
   allocated region it will use up.
*/

static int SetOverlap( list2 )
xpandList *list2;
{
  xpandList /*int*/ list;

  ListCreate( list, int, 4 );
  ListAddItem( list, int, 80 );
  ListAddItem( list, int, 60 );
  ListAddItem( list, int, 40 );

  *list2 = list;

  return 0;
}



static int StateOnScreen( tl, type, proc )
timeLineInfo *tl;
int type, proc;
{
  /* return 0 if the state is definitely not visible, 1 if it
     may be partially visible */

  /* for now, return everything as visible */

  return 1;

}





static int TimeLineDrawState( data_v, idx, type, proc, startTime, endTime,
			      parent, firstChild, overlapLevel )
void *data_v;
int idx;
int type, proc, parent, firstChild, overlapLevel;
double startTime, endTime;
{
  static int update_count = 0;

  char *argv[20], tags[50];
  char nums[4][50];		/* temp storage for stringized numbers */

  timeLineInfo *tl;
  double l, r, t, b;
  int visibleProcIdx, firstChildCanvasId;
    /* corners of the state: left, right, top, bottom */
  char cmd[1000];
  tl_stateInfo state_info;
  char *state_name, *state_color, *state_bitmap;
  logData *log_copy;

  Tcl_CmdInfo cmd_info;

    /* nothing special to do after the last state */
  if (idx == -1) return 0;

    /* reset the counter if this is a new logfile */
  if (idx == 0) update_count = 0;

#if DEBUG>1
  fprintf( stderr, "TimeLineDrawState sent state #%d.\n", idx );
  fprintf( stderr, "%f to %f, proc %d, type %d, overlapLevel %d\n",
	   startTime, endTime, proc,
	   type, overlapLevel );
#endif


  tl = (timeLineInfo *) data_v;

  if (StateOnScreen( tl, type, proc )) {

    Log_GetStateDef( tl->log, type, &state_name, &state_color,
		     &state_bitmap );

      /* get the first child's canvas Id */
    firstChildCanvasId = (firstChild == -1) ? -1 :
      ListItem( tl->stateList, tl_stateInfo, firstChild ).canvasId1;

      /* convert process number to its index in the list of VISIBLE
	 processes */
    visibleProcIdx = Vis_x2i( tl->procVis, proc );

    l = Time2Pix( tl, startTime );
    r = Time2Pix( tl, endTime );
      /* the overlap.halfWidth is a number from 0 to 100; how much
	 of the alloted space to fill up */
    t = Proc2Pix( tl, visibleProcIdx + .5 * (100 -
      ListItem( tl->overlap.halfWidths, int, overlapLevel ))/100.0 );
    b = Proc2Pix( tl, visibleProcIdx + .5 + .5 * 
      ListItem( tl->overlap.halfWidths, int, overlapLevel )/100.0 );

      /* get data necessary for calling the canvas' command directly */
      /* to elimiate time spent parsing arguments, which is significant, */
      /* at least on my machine */

    Tcl_GetCommandInfo( tl->interp, tl->canvasName, &cmd_info );

    sprintf( nums[0], "%.17g", l );
    sprintf( nums[1], "%.17g", t );
    sprintf( nums[2], "%.17g", r );
    sprintf( nums[3], "%.17g", b );

    argv[0] = tl->canvasName;
    argv[1] = "create";
    argv[2] = "rect";
    argv[3] = nums[0];
    argv[4] = nums[1];
    argv[5] = nums[2];
    argv[6] = nums[3];
    argv[7] = "-tags";
    /*
       If this is color, we want this to be the main rectangle, thus
       the tag should be color_%d.  If this is b&w, this rectangle is
       the coverup rectangle, and should be color_bg.

       Unfortunately, when it comes time to print the color ones in
       b&w, they again need the masking rectangle.  Grrr.
    */
    argv[8] = "color_bg";
    argv[9] = "-fill";
    argv[10] = tl->bg;
    argv[11] = 0;

/*
fprintf( stderr, "TimeLineDrawState: %s %s %s %s %s %s %s %s %s %s %s\n",
	 argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6],
	 argv[7], argv[8], argv[9], argv[10] );
*/
      
      /* call the canvas command manually */
    Tcl_CallArgv( cmd_info, tl->interp, 11, argv );
    
      /* get canvas id */
    sscanf( tl->interp->result, "%d", &state_info.canvasId1 );

    sprintf( tags, "color_%d", type );

    argv[8] = tags;
    if (tl->bw) {
      /* argv[9] = "-fill"; */
      argv[10] = tl->outlineColor;
      argv[11] = "-stipple";
      argv[12] = state_bitmap;
      argv[13] = "-outline";
      argv[14] = tl->outlineColor;
      argv[15] = 0;
    
        /* call the canvas command manually */
      Tcl_CallArgv( cmd_info, tl->interp, 15, argv );

    } else {
      /* argv[9] = "-fill"; */
      argv[10] = state_color;
      argv[11] = "-outline";
      argv[12] = tl->outlineColor;
      argv[13] = 0;
    
        /* call the canvas command manually */
      Tcl_CallArgv( cmd_info, tl->interp, 13, argv );
    }

      /* get canvas id */
    sscanf( tl->interp->result, "%d", &state_info.canvasId2 );

      /* if this state has children, drop it below their rectangles */
    if (firstChildCanvasId != -1) {

      sprintf( nums[0], "%d", state_info.canvasId2 );
      sprintf( nums[1], "%d", firstChildCanvasId );
      argv[1] = "lower";
      argv[2] = nums[0];
      argv[3] = nums[1];
      argv[4] = 0;

        /* drop this rectangle below its first child */
      Tcl_CallArgv( cmd_info, tl->interp, 4, argv );

      sprintf( nums[1], "%d", state_info.canvasId1 );
      argv[2] = nums[1];
      argv[3] = nums[0];

        /* drop coverup rectangle below the real rectangle that was
	   just put beneath its first child */
      Tcl_CallArgv( cmd_info, tl->interp, 4, argv );
    }

  } else {

    /* if the state is not visible */
    state_info.canvasId1 = TL_STATE_NOT_VISIBLE;
  }

  ListAddItem( tl->stateList, tl_stateInfo, state_info );
  log_copy = tl->log;

#if 0
    /* just to keep things moving */
    /* leave out TK_IDLE_EVENTS since we want to give everything a chance
       except the canvas redraw */
  while (Tk_DoOneEvent( TK_DONT_WAIT | TK_X_EVENTS | TK_FILE_EVENTS |
		        TK_TIMER_EVENTS )) {
  }
#endif


  update_count++;
  if (update_count == UPDATES_EVERY_N_STATES) {
    strcpy( cmd, "update" );

      /* right here, we might get deleted.  Better watch out.  */
    Tcl_Eval( tl->interp, cmd );

    update_count = 0;
  }

  return Log_Halted( log_copy );
}


#if DEBUG
static int PrintStates( tl )
timeLineInfo *tl;
{
  int i, n;
  tl_stateInfo *state;

  state = ListHeadPtr( tl->stateList, tl_stateInfo );
  n = ListSize( tl->stateList, tl_stateInfo );

  for (i=0; i<n; i++,state++) {
    fprintf( stderr, "%d\n", state->canvasId1 );
  }
  return TCL_OK;
}
#endif



  /* get the current item from the canvas, figure out what kind of
     object it is (state, event, or message) and return the type
     and index of the item, for example, {state 35} */


static int CurrentItem( data, interp, argc, argv )
ClientData data;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  int i, n;
  int canvas_id;
  char cmd[TMP_CMD_LEN];
  timeLineInfo *tl;
  tl_stateInfo *statePtr;

  if (argc != 2) {
    Tcl_AppendResult( interp, "wrong # of arguments: Timeline_CurrentItem ",
		      "<win>", (char*)0 );
    return TCL_ERROR;
  }

  tl = TlPtrFromWindow( interp, argv[1] );
  if (!tl) {
    Tcl_AppendResult( interp, "window ", argv[1], " is not a timeline.",
		      (char*)0 );
    return TCL_ERROR;
  }

  sprintf( cmd, "%s find withtag current", tl->canvasName );
  if (TCL_OK != Tcl_Eval( tl->interp, cmd )) {
    return TCL_ERROR;
  }

    /* if the result string is empty, there is no current item,
       so return an empty string (leave result alone). */
  if (tl->interp->result[0] == 0) return TCL_OK;

    /* get the canvas id */
  sscanf( tl->interp->result, "%d", &canvas_id );

  n = ListSize( tl->stateList, tl_stateInfo );
  statePtr = ListHeadPtr( tl->stateList, tl_stateInfo );
  for (i=0; i<n; i++,statePtr++) {
    if (canvas_id == statePtr->canvasId2) {
        /* interp->result is supposed to point to about 200 bytes
	   of useable memory, so we can use it */
      sprintf( tl->interp->result, "state %d", i );
      return TCL_OK;
    }
  }

    /* if the id does not match any item, return empty string */
  tl->interp->result[0] = 0;

  return TCL_OK;
}


#if 0

static int UpdateStateList( tl, list, n )
timeLineInfo *tl;
int *list, n;
{
  int i, type, proc, parent, firstChild, overlapLevel;
  double startTime, endTime;
  tl_stateInfo *state;

  for (i=0; i < n; i++) {
      /* if we're beyond the list of drawn states, stop */
    if (list[i] >= ListSize( tl->stateList, tl_stateInfo )) {
      break;
    }

      /* get handy pointer to the state */
    state = ListHeadPtr( tl->stateList, tl_stateInfo ) + list[i];

      /* if the state has already been drawn, skip it */
    if (state->canvasId1 < 0) {

        /* if not visible, draw it */
      if (state->canvasId1 == TL_STATE_NOT_VISIBLE) {
	  /* get stats on the state */
	Log_GetState( tl->log, list[i], &type, &proc, &startTime, &endTime,
		      &parent, &firstChild, &overlapLevel );
	
#if DEBUG_UPDATE
	fprintf( stderr, "drawing state %d from %f to %f\n",
		 list[i], startTime,
		 endTime );
#endif

	  /* draw it */
	TimeLineDrawState( (void*)tl, list[i], type, proc, startTime,
			  endTime, parent, firstChild, overlapLevel );
      }
    }
  }

  return 0;
}

#endif
