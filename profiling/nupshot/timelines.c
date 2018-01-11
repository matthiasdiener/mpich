#include <stdlib.h>
#include "tcl.h"
#include "tk.h"
#include "log.h"
#include "log_widget.h"
#include "timelines.h"
#include "tclptr.h"
#include "str_dup.h"
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
#define DEBUG_RESIZE 0

  /* length to use for temporary Tcl commands */
#define TMP_CMD_LEN 200

  /* macro for calling a Tcl function manually, checking the return,
     and printing an error message on failure.  No return, no exit(). */

#define TRY_TCL_CMD_ARGV( cmd_info, interp, argc, argv ) do {\
  if (TCL_OK != (*(cmd_info).proc)( (cmd_info).clientData, (interp), \
                               (argc), (argv) )) { \
    fprintf( stderr, "TCL error in %s, line %d: %s\n", __FILE__, \
             __LINE__, (interp)->result ); \
  } \
} while (0)



#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

/*
   The function that is called by the tcl routine "timeline".
   Widget creation command.  clientData is (Tk_Window)mainWin.
*/
static int Timeline_create ARGS((ClientData  clientData, Tcl_Interp *interp,
			   int argc, char *argv[]));

/*
   Event handler
*/
static void TimeLineEvents ARGS(( ClientData data, XEvent *eventPtr ));

/*
   Function that we can tell Tk to call at the next update.
   This will check the the whenIdle field to know what needs
   to be redrawn or updated, if anything.
*/
static void TimeLineWhenIdle ARGS(( ClientData ));

/*
   Handle all configuration stuff, including calling Tk_ConfigureWidget.
*/
static int Configure ARGS(( timeLineInfo *tl, int argc, char **argv ));

/*
   Called when the window has been told it's been resized.
   Should scale everything to that the display seems the same, just
   stretched.
*/
static int Resize ARGS(( timeLineInfo *tl ));

/*
   Check what objects need to created if they are visible and not created yet
*/
static int UpdateVisibleRegion ARGS(( timeLineInfo *tl, int flags ));

/*
   Zoom with the given factors in/out at the given point.  Factor > 1
   will be a zoom in.
*/
static int Zoom ARGS(( timeLineInfo *tl, double x, double y,
                       double factor_x, double factory_y ));

/*
   Zoom with the given factor in/out at the given point.  Factor > 1
   will be a zoom in.
*/
static int ZoomTime ARGS(( timeLineInfo *tl, double x, double factor ));

/*
   Zoom with the given factor in/out at the given point.  Factor > 1
   will be a zoom in.
*/
static int ZoomProc ARGS(( timeLineInfo *tl, double y, double factor ));

/*
   Move the view to that the spcified time is at the far left side.
*/
static int ViewTime ARGS(( timeLineInfo*, double from_time, double to_time ));

/*
   Move/scale the view so that the specified time range is in view.
*/
static int ViewFromTime ARGS(( timeLineInfo*, double from_time ));

/*
   Raw canvas canvasx command
*/
static int CanvasxCmd ARGS((timeLineInfo*, double ));

/*
   Raw canvas xview command.  Units are pixels/10
*/
static int Xview ARGS((timeLineInfo *tl, int i ));

/*
   Return the type and index of whatever item the cursor is currently over.
   For example:
      set item [.t currentitem]
        # item = "state 35")
      puts [logfile $l [lindex $item 0] [lindex $item 1]]
*/
int CurrentItem ARGS(( timeLineInfo *tl ));

/*
static int CanvasSize( char *canvas,
		       int *width, int *height );
*/
static int SetOverlap ARGS(( xpandList /*int*/ *list2 ));
static int StateOnScreen ARGS(( timeLineInfo *tl, int type, int proc,
			        double startTime, double endTime ));

/*
   ResetBounds - recalculate {top,bottom}Proc and {left,right}time
*/
static int ResetBounds ARGS(( timeLineInfo *tl ));

/*
   Convert between time (in seconds), process numbers (floating-point,
   n.5 = center of timeline), and pixel coords.  Everything is in
   doubles.
*/
static double Time2Pix ARGS(( timeLineInfo *tl, double time ));
static double Pix2Time ARGS(( timeLineInfo *tl, double pix ));
static double Proc2Pix ARGS(( timeLineInfo *tl, double proc ));
static double Pix2Proc ARGS(( timeLineInfo *tl, double pix ));

/*
   Tcl wrappers for conversion routines
*/
static int Time2PixCmd ARGS(( timeLineInfo*, double ));
static int Pix2TimeCmd ARGS(( timeLineInfo*, double ));

/*
   Draw, or at least register one state.
   The void *data is a timeLineInfo*.
*/
static int TimeLineDrawState ARGS(( void *data, int idx, int type, int proc,
				    double startTime, double endTime,
				    int parent, int firstChild,
				    int overlapLevel ));

/*
   The function that is called by Tcl scripts for all the timeline
   widget commands.  The clientData is a timeLineInfo*.
*/
static int TimeLineCmd ARGS(( ClientData, Tcl_Interp *, int argc,
			      char **argv ));

/*
   Passed to Tcl_CmdDeleteProc, closes out the widget record.
*/
static void CloseTimeLineCmd ARGS(( ClientData clientData ));

#if DEBUG
static int PrintStates ARGS(( timeLineInfo* ));
#endif



#define DEF_OUTLINE_COLOR "white"
#define DEF_OUTLINE_MONO  "black"
#define DEF_MSG_COLOR "black"
#define DEF_MSG_MONO "black"
#define DEF_MSG_COLOR "black"
#define DEF_MSG_MONO "black"
#define DEF_BG_COLOR "black"
#define DEF_BG_MONO  "white"
#define DEF_TIMELINE_COLOR "red"
#define DEF_TIMELINE_MONO  "black"
#define DEF_X_SCROLL_COMMAND ""
#define DEF_Y_SCROLL_COMMAND ""
#define UPDATES_EVERY_N_STATES 200
#define DEF_WIDTH "500"
#define DEF_HEIGHT "200"
#define DEF_VIS_TIME "-1"
   /* will be filled in with the full time range */
#define DEF_LEFT_TIME "-1"
   /* will be filled in the with leftmost time */
#define DEF_VIS_PROCS "-1"
   /* will be filled in the with the number of visible processes */
#define DEF_TOP_PROC "0"
#define DEF_IS_BW "0"
#define DEF_IS_COLOR "0"


static Tk_ConfigSpec configSpecs[] = {
  {TK_CONFIG_PIXELS, "-width", "width", "width",
     DEF_WIDTH, Tk_Offset( timeLineInfo, visWidth ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_PIXELS, "-height", "height", "height",
     DEF_HEIGHT, Tk_Offset( timeLineInfo, visHeight ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_DOUBLE, "-vistime", "vistime", "time",
     DEF_VIS_TIME, Tk_Offset( timeLineInfo, visTime ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_DOUBLE, "-lefttime", "to", "time",
     DEF_LEFT_TIME, Tk_Offset( timeLineInfo, leftTime ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_DOUBLE, "-visprocs", "visprocs", "proc",
     DEF_VIS_PROCS, Tk_Offset( timeLineInfo, visProcs ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_DOUBLE, "-topproc", "topproc", "proc",
     DEF_TOP_PROC, Tk_Offset( timeLineInfo, topProc ),
     0, (Tk_CustomOption*)0},

  {TK_CONFIG_STRING, "-outline", "outline", "outline",
     DEF_OUTLINE_COLOR, Tk_Offset( timeLineInfo, outlineColor ),
     TK_CONFIG_COLOR_ONLY, (Tk_CustomOption*)NULL},
  {TK_CONFIG_STRING, "-outline", "outline", "outline",
     DEF_OUTLINE_MONO, Tk_Offset( timeLineInfo, outlineColor ),
     TK_CONFIG_MONO_ONLY, (Tk_CustomOption*)NULL},

  {TK_CONFIG_STRING, "-msgcolor", "msgcolor", "msgcolor",
     DEF_MSG_COLOR, Tk_Offset( timeLineInfo, msgColor ),
     TK_CONFIG_COLOR_ONLY, (Tk_CustomOption*)NULL},
  {TK_CONFIG_STRING, "-msgcolor", "msgcolor", "msgcolor",
     DEF_MSG_MONO, Tk_Offset( timeLineInfo, msgColor ),
     TK_CONFIG_MONO_ONLY, (Tk_CustomOption*)NULL},

  /* The foreground color of a timeline is the color of its timelines */

  {TK_CONFIG_STRING, "-foreground", "foreground", "Foreground",
     DEF_TIMELINE_COLOR, Tk_Offset( timeLineInfo, lineColor ),
     TK_CONFIG_COLOR_ONLY, (Tk_CustomOption*)NULL},
  {TK_CONFIG_STRING, "-foreground", "foreground", "Foreground",
     DEF_TIMELINE_MONO, Tk_Offset( timeLineInfo, lineColor ),
     TK_CONFIG_MONO_ONLY, (Tk_CustomOption*)NULL},

  {TK_CONFIG_SYNONYM, "-fg", "foreground", (char*)0,
     (char*)0, 0, 0, (Tk_CustomOption*)0},

  {TK_CONFIG_STRING, "-background", "background", "Background",
     DEF_BG_COLOR, Tk_Offset( timeLineInfo, bg ),
     TK_CONFIG_COLOR_ONLY, (Tk_CustomOption*)NULL},
  {TK_CONFIG_STRING, "-background", "background", "Background",
     DEF_BG_MONO, Tk_Offset( timeLineInfo, bg ),
     TK_CONFIG_MONO_ONLY, (Tk_CustomOption*)NULL},

  {TK_CONFIG_SYNONYM, "-bg", "background", (char*)0,
     (char*)0, 0, 0, (Tk_CustomOption*)0},

  {TK_CONFIG_STRING, "-xscrollcommand", "xScrollCommand", "ScrollCommand",
     DEF_X_SCROLL_COMMAND, Tk_Offset( timeLineInfo, xscrollCommand ),
     0, (Tk_CustomOption*)NULL},

  {TK_CONFIG_STRING, "-yscrollcommand", "yScrollCommand", "ScrollCommand",
     DEF_Y_SCROLL_COMMAND, Tk_Offset( timeLineInfo, yscrollCommand ),
     0, (Tk_CustomOption*)NULL},

  {TK_CONFIG_BOOLEAN, "-bw", "depth", "depth",
     DEF_IS_BW, Tk_Offset( timeLineInfo, is_bw ),
     0, (Tk_CustomOption*)NULL},

  {TK_CONFIG_BOOLEAN, "-color", "depth", "depth",
     DEF_IS_COLOR, Tk_Offset( timeLineInfo, is_color ),
     0, (Tk_CustomOption*)NULL},

  {TK_CONFIG_END, (char*)0, (char*)0, (char*)0,
     (char*)0, 0, 0, (Tk_CustomOption*)NULL}
};




int timelineAppInit( interp )
Tcl_Interp *interp;
{
  Tcl_CreateCommand( interp, "timeline", Timeline_create,
		     (ClientData)Tk_MainWindow( interp ),
		     (Tcl_CmdDeleteProc*) 0 );
  return 0;
}




static int Timeline_create( clientData, interp, argc, argv )
ClientData clientData;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  int i, n;
  logDataAndFile *log;
  timeLineInfo *tl;
  char *logToken, *windowName;
  char cmd[200];

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
  tl->log = &log->data;

    /* create the Tk window */
  tl->win = Tk_CreateWindowFromPath( interp, (Tk_Window)clientData,
				     tl->windowName, (char *)0 );
  if (!tl->win) {
    Tcl_AppendResult( interp, "Failed to create window ",
		      tl->windowName, (char*)0 );
    goto failed_2;
  }

    /* set colors, visible region, and scroll commands, skipping over command
       name and the standard arguments */
  if (TCL_OK != Configure( tl, argc-3, argv+3 )) {
    goto failed_2;
  }
    /* after this, visTime, leftTime, visProcs, topProc, 
       visWidth, and visHeight will be set */

    /* check for really small canvases */
  if (tl->visWidth <= 0 || tl->visHeight <= 0) {
    sprintf( tl->interp->result, "Invalid size: %d x %d",
	     tl->visWidth, tl->visHeight );
    goto failed_2;
  }

    /* set the total width based on the visible width and the total
       amount of time that needs to be represented */
  tl->width = tl->visWidth *	/* total width of the widget */
              (Log_EndTime( tl->log ) - Log_StartTime( tl->log )) /
                                /* total length of logfile */
              tl->visTime;      /* how much time shown onscreen */

    /* set the total height based on the number of processes that have
       been selected to be visible and the number that can be shown
       on screen at a time */
  tl->height = tl->visHeight *	      /* height of the widget */
               /* Vis_n(tl->procVis)*/
               Log_Np( tl->log ) /    /* number of visible processes */
				      
	       tl->visProcs;	      /* max # of processes onscreen */

  tl->visLeft = 0;
  tl->visTop = 0;

    /* set scroll region */
  tl->farTop = 0 - tl->topProc / Log_Np( tl->log ) * tl->height;
  tl->farBottom = tl->farTop + tl->height;
  tl->farLeft = 0 - (tl->leftTime - Log_StartTime( tl->log )) /
	        (Log_EndTime( tl->log ) - Log_StartTime( tl->log )) *
                tl->width;
  tl->farRight = tl->farLeft + tl->width;

    /* set uppermost visible coordinate */
  tl->visTop = tl->topProc / tl->visProcs * tl->height;

  tl->xresizeFactor = 1.0;
  tl->yresizeFactor = 1.0;

    /* tell Tcl I want all resize events */
  Tk_CreateEventHandler( tl->win, (unsigned long)(StructureNotifyMask),
			 TimeLineEvents, (ClientData)tl );

    /* call Tcl to create the canvas */
  sprintf( cmd, "canvas %s -width %d -height %d -bg %s", tl->canvasName,
	   tl->visWidth, tl->visHeight, tl->bg );
    /* and return any error code */
  if (TCL_OK != Tcl_Eval( interp, cmd )) goto failed_3;

    /* pack the canvas inside the window */
  sprintf( cmd, "pack %s -expand 1 -fill both", tl->canvasName );
    /* and return any error code */
  if (TCL_OK != Tcl_Eval( interp, cmd )) goto failed_3;


#if DEBUG_UPDATE_GOOFINESS>1
  fprintf( stderr, "break ??0\n" );
#endif
#if DEBUG_UPDATE_GOOFINESS
  sprintf( cmd, "update" );
  if (TCL_OK != Tcl_Eval( interp, cmd )) goto failed_3;
#endif
#if DEBUG_UPDATE_GOOFINESS>1
  fprintf( stderr, "break ??1\n" );
#endif

    /* create visiblity definitions such that everything is visible */
    /* might want to declare a special case for the visible object for
       the sake of speed in which everything is visible, no checking
       needed */
  tl->procVis = Vis_new();
  for (i=0, n=Log_Np( tl->log ); i < n; i++) {
    Vis_add( tl->procVis, i, -1 );
  }

  tl->eventVis = Vis_new();
  for (i=0, n=Log_Nevents( tl->log ); i < n; i++) {
    Vis_add( tl->eventVis, i, -1 );
  }

  tl->stateVis = Vis_new();
  for (i=0, n=Log_Nstates( tl->log ); i < n; i++) {
    Vis_add( tl->stateVis, i, -1 );
  }

  tl->msgVis = Vis_new();
  for (i=0, n=Log_Nmsgs( tl->log ); i < n; i++) {
    Vis_add( tl->msgVis, i, -1 );
  }


    /* get the list of overlap levels, you know, how wide to draw
       a state bar if is is overlapping X other states */
  SetOverlap( &tl->overlap.halfWidths );


    /* precompute the boundaries of the screen in times and processes */
  ResetBounds( tl );

#if DEBUG
  fprintf( stderr, "scroll_region: %d %d %d %d, visible region: %d %d %d %d, native units: %f %f %f %f, width %d, height %d\n",
	   tl->farLeft, tl->farTop, tl->farRight, tl->farBottom,
	   tl->visWidth, tl->visHeight, tl->visLeft, tl->visTop,
	   tl->topProc, tl->leftTime, tl->bottomProc, tl->rightTime,
	   tl->width, tl->height );
  fprintf( stderr, "time range: %f to %f\n",
	   Log_StartTime( tl->log), Log_EndTime( tl->log) );
  fprintf( stderr, "np: %d\n", Log_Np( tl->log) );
#endif
  
    /* set the canvas' scroll region */
  sprintf( cmd, "%s config -scrollregion [list %d %d %d %d]", tl->canvasName,
	   (int)(tl->farLeft), (int)(tl->farTop),
	   (int)(tl->farRight), (int)(tl->farBottom) );
  if (Tcl_Eval( interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* set the canvas' scroll commands */
  sprintf( cmd, "%s config -xscrollcommand {%s}", tl->canvasName,
	   tl->xscrollCommand );
  if (Tcl_Eval( interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* set the canvas' scroll commands */
  sprintf( cmd, "%s config -yscrollcommand {%s}", tl->canvasName,
	   tl->yscrollCommand );
  if (Tcl_Eval( interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* bind the second mouse button to scan the canvas */
  sprintf( cmd, "bind %s <ButtonPress-2>  {%s scan mark %%x %%y}",
	   tl->canvasName, tl->canvasName );
  if (Tcl_Eval( interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }
  sprintf( cmd, "bind %s <Button2-Motion>  {%s scan dragto %%x %%y}",
	   tl->canvasName, tl->canvasName );
  if (Tcl_Eval( interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* create list for for state drawing information */
    /* don't forget we still need to add messages and events */
  ListCreate( tl->stateList, tl_stateInfo, 100 );

    /* add this timeline display to the list of function to be called
       whenever a new state is read */
  tl->drawStateToken = 
    Log_AddDrawState( tl->log, TimeLineDrawState, (void*)tl );
  
    /* create the widget command for this window */
  Tcl_CreateCommand( interp, tl->windowName, TimeLineCmd,
     (ClientData)tl, CloseTimeLineCmd );


    /* like, wow, no errors! */
  return TCL_OK;

 failed_3:
  Tk_DestroyWindow( tl->win );
 failed_2:
  free( tl->windowName );
  free( tl->canvasName );
  free( tl );
 failed_1:
  return TCL_ERROR;
}


static int Configure( tl, argc, argv )
timeLineInfo *tl;
int argc;
char **argv;
{
    /* clear these just to Tcl doesn't try to free() them */
  tl->outlineColor = tl->lineColor = tl->msgColor = 0;
  tl->bg = tl->xscrollCommand = tl->yscrollCommand = 0;

  if (TCL_OK != Tk_ConfigureWidget( tl->interp, tl->win, configSpecs,
				    argc, argv, (char*)tl, 0 )) {
    fprintf( stderr, "%s, %d: %s\n", __FILE__, __LINE__,
	     tl->interp->result );
    return TCL_ERROR;
  }

    /* if visTime is still -1, set it to the complete range */
  if (tl->visTime == -1) {
    tl->visTime = Log_EndTime( tl->log ) - Log_StartTime( tl->log );
  }

    /* if not specified, set display to show all processes */
  if (tl->visProcs == -1) {
    tl->visProcs = Log_Np( tl->log );
  }

    /* if not specified, set start time to the beginning of the log */
  if (tl->leftTime == -1) {
    tl->leftTime = Log_StartTime( tl->log );
  }

    /* set whether to draw in black&white or not */
    /* if neither -bw or -color is set, or both are set, figure it out
       for myself based on the depth of the window's colormap */

  tl->bw = (tl->is_bw == tl->is_color) ? (Tk_Depth( tl->win )==1) : tl->is_bw;

#if DEBUG
  fprintf( stderr, "bg:%s, outlineColor:%s, msgColor:%s, lineColor:%s\n",
	   tl->bg, tl->outlineColor, tl->msgColor, tl->lineColor );
#endif

  return 0;

}



/*
 * TimeLineEvents
 *
 * Should be sent ConfigureNotify and DestroyNotify events.
 *
 */

static void TimeLineEvents( clientData, event )
ClientData clientData;
XEvent *event;
{
  timeLineInfo *tl;
  
  tl = (timeLineInfo*) clientData;
  
#if DEBUG_EVENTS
  fprintf( stderr, "TimeLineEvents - tl = %s,%s,%d,%d,%f,%f\n",
	   tl->windowName, tl->canvasName, tl->width, tl->height,
	   tl->xresizeFactor, tl->yresizeFactor );
#endif

  switch (event->type) {

  case DestroyNotify:
#if DEBUG_DESTROY
    fprintf( stderr, "DestroyNotify received, destroying timeline\n" );
#endif
    CloseTimeLineCmd( clientData );
    break;

  case ConfigureNotify:
    tl->xresizeFactor = tl->xresizeFactor * event->xconfigure.width /
      tl->visWidth;
    tl->yresizeFactor = tl->yresizeFactor * event->xconfigure.height /
      tl->visHeight;
    tl->whenIdle |= TL_NEEDS_RESIZE;
#if DEBUG_RESIZE
    fprintf( stderr, "Resize to %d x %d from %d x %d (total %d x %d\n",
	     event->xconfigure.width,
	     event->xconfigure.height, tl->visWidth,
	     tl->visHeight, tl->width, tl->height );
#endif
    Tk_DoWhenIdle( TimeLineWhenIdle, (ClientData)tl );
    break;

  }

}



static void TimeLineWhenIdle( data )
ClientData data;
{
  timeLineInfo *tl;
  tl = (timeLineInfo*) data;

  if (tl->whenIdle & TL_NEEDS_RESIZE) {
    Resize( tl );
  }

  tl->whenIdle = 0;

}



static int Resize( tl )
timeLineInfo *tl;
{

#if DEBUG_RESIZE
  fprintf( stderr, "Resizing by a factor of %f x %f\n",
	   tl->xresizeFactor, tl->yresizeFactor );
#endif
  if (tl->xresizeFactor == 1.0 && tl->yresizeFactor == 1.0) return 0;
  
  tl->visWidth *= tl->xresizeFactor;
  tl->visHeight *= tl->yresizeFactor;
  
  if (TCL_OK != Zoom( tl, tl->leftTime, tl->topProc,
		      tl->xresizeFactor, tl->yresizeFactor )) {
    fprintf( stderr, "%s\n", tl->interp->result );
  } else {
    tl->xresizeFactor = tl->yresizeFactor = 1.0;

/*
    xview = (tl->visLeft - tl->farLeft)/10;
    if (TCL_OK != Xview( tl, xview )) {
      fprintf( stderr, "%s\n", tl->interp->result );
    }
*/

  }

  return 0;
}




static void CloseTimeLineCmd( data )
ClientData data;
{
  timeLineInfo *tl;

    /* convert data pointer to my widget record pointer */
  tl = (timeLineInfo *)data;

  free( tl->windowName );	/* STRDUP'd */
  free( tl->canvasName );		/* malloc()'d */
  free( tl );			/* malloc()'d */
}


static int TimeLineCmd( clientData, interp, argc, argv )
ClientData clientData;
Tcl_Interp *interp;
int argc;
char *argv[];
{
  timeLineInfo *tl;
  double x, factor, from_time, to_time;
  int i;

  tl = (timeLineInfo*)clientData;

  if (!strcmp( argv[1], "zoom_time" )) {
    if (TCL_OK != ConvertArgs( interp,
      "<timeline> zoom_time <time> <factor>",
      "2 ff", argc, argv, &x, &factor )) {
      return TCL_ERROR;
    }

    return ZoomTime( tl, x, factor );
  }

  if (!strcmp( argv[1], "canvasx" )) {
    if (TCL_OK != ConvertArgs( interp,
      "<timeline> canvasx <coord>", "2 f", argc, argv, &x )) {
      return TCL_ERROR;
    }

    return CanvasxCmd( tl, x );
  }

  if (!strcmp( argv[1], "pix2time" )) {
    if (TCL_OK != ConvertArgs( interp,
      "<timeline> pix2time <coord>", "2 f", argc, argv, &x )) {
      return TCL_ERROR;
    }

    return Pix2TimeCmd( tl, x );
  }

  if (!strcmp( argv[1], "time2pix" )) {
    if (TCL_OK != ConvertArgs( interp,
      "<timeline> time2pix <coord>", "2 f", argc, argv, &x )) {
      return TCL_ERROR;
    }

    return Time2PixCmd( tl, x );
  }

  if (!strcmp( argv[1], "xview" )) {
    if (TCL_OK != ConvertArgs( interp,
      "<timeline> xview <coord>", "2 d", argc, argv, &i )) {
      return TCL_ERROR;
    }

    return Xview( tl, i );
  }

  if (!strcmp( argv[1], "view_time" )) {
    if (argc == 4) {
      if (TCL_OK != ConvertArgs( interp,
	  "<timeline> view_time <from time> [<to time>]",
			      "2 ff", argc, argv, &from_time, &to_time )) {
	return TCL_ERROR;
      } else {
	return ViewTime( tl, from_time, to_time );
      }
    } else if (argc == 3) {
      if (TCL_OK != ConvertArgs( interp,
	  "<timeline> view_time <from time> [<to time>]",
			      "2 f", argc, argv, &from_time )) {
	return TCL_ERROR;
      } else {
	return ViewFromTime( tl, from_time );
      }
    } else {
      Tcl_SetResult( interp, "Wrong # of args to view_time", TCL_STATIC );
      return TCL_ERROR;
    }
  }

#if DEBUG
  if (!strcmp( argv[1], "printstates" )) {
    return PrintStates( tl );
  }
#endif

    /* return type and index of a canvas item, for example: {state 35} */
    /* return empty string if no matching item found */
  if (!strcmp( argv[1], "currentitem" )) {
    return CurrentItem( tl );
  }

  Tcl_AppendResult( interp, "unrecognized command.  one of:",
		    "zoom_time, canvasx, pix2time, time2pix, xview, ",
		    "view_time", (char*) 0 );

  return TCL_ERROR;
}



/*
   Zoom the canvas by a given factor, at a given time and process point
*/

static int Zoom( tl, time, proc, time_factor, proc_factor )
timeLineInfo *tl;
double time, proc, time_factor, proc_factor;
{
  if (TCL_OK != ZoomTime( tl, time, time_factor ) ||
      TCL_OK != ZoomProc( tl, proc, proc_factor ) ) {
    return TCL_ERROR;
  } else {
    return TCL_OK;
  }
}




/*
   zoom the canvas by a given factor, at a given time
*/

static int ZoomTime( tl, time, factor )
timeLineInfo *tl;
double time, factor;
{
  double x;
  char cmd[200];


    /* convert the given time to canvas pixel units */
  x = Time2Pix( tl, time );

    /* make sure we won't zoom out of bounds */
  /* AdjustZoomBounds( tl, &x, &factor ); */
    /* Actually, no.  Since everyone would have to do this,
       have the master take care of this before calling each
       display's zoom_time command. */


    /* build the canvas command */
  sprintf( cmd, "%s scale all %.17g 0 %.17g 1", tl->canvasName, x,
	  factor );
    /* execute the canvas command */
  if (Tcl_Eval( tl->interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* update internal boundaries */
  tl->width    *= factor;
  tl->farLeft   = x - factor * (x - tl->farLeft);
  tl->farRight  = x + factor * (tl->farRight - x);
  ResetBounds( tl );

#if DEBUG_ZOOM
  fprintf( stderr, "reset scrollregion to %d %d %d %d\n",
	   tl->farLeft, tl->farTop,
	   tl->farLeft+tl->width,
	   tl->farTop+tl->height );
#endif

    /* update the canvas' scroll region */
  sprintf( cmd, "%s config -scrollregion [list %d %d %d %d]", tl->canvasName,
	   tl->farLeft, tl->farTop,
	   tl->farLeft+tl->width,
	   tl->farTop+tl->height );
  if (Tcl_Eval( tl->interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

  return TCL_OK;
}


/*
   zoom the canvas by a given factor, at a given process point
*/

static int ZoomProc( tl, proc, factor )
timeLineInfo *tl;
double proc, factor;
{
  double y;
  char cmd[200];


    /* convert the given process to canvas pixel units */
  y = Proc2Pix( tl, proc );

    /* make sure we won't zoom out of bounds */
  /* AdjustZoomBounds( tl, &x, &factor ); */
    /* Actually, no.  Since everyone would have to do this,
       have the master take care of this before calling each
       display's zoom_time command. */


    /* build the canvas command */
  sprintf( cmd, "%s scale all 0 %.17g 1 %.17g", tl->canvasName, y,
	  factor );
    /* execute the canvas command */
  if (Tcl_Eval( tl->interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

    /* update internal boundaries */
  tl->height   *= factor;
  tl->farTop    = y - factor * (y - tl->farTop);
  tl->farBottom = y + factor * (tl->farBottom - y);
  ResetBounds( tl );

#if DEBUG_ZOOM
  fprintf( stderr, "reset scrollregion to %d %d %d %d\n",
	   tl->farLeft, tl->farTop,
	   tl->farLeft+tl->width,
	   tl->farTop+tl->height );
#endif

    /* update the canvas' scroll region */
  sprintf( cmd, "%s config -scrollregion [list %d %d %d %d]", tl->canvasName,
	   tl->farLeft, tl->farTop,
	   tl->farLeft+tl->width,
	   tl->farTop+tl->height );
  if (Tcl_Eval( tl->interp, cmd ) != TCL_OK) {
    return TCL_ERROR;
  }

  return TCL_OK;
}


/* convert screen x or y to canvas x or y */
static int CanvasxCmd( tl, x )
timeLineInfo *tl;
double x;
{
  char cmd[200];

  sprintf( cmd, "%s canvasx %.17g", tl->canvasName, x );
  return Tcl_Eval( tl->interp, cmd );
}


/* convert canvas coordinate to time */
static int Time2PixCmd( tl, time )
timeLineInfo *tl;
double time;
{
  sprintf( tl->interp->result, "%.17g", Time2Pix( tl, time ) );

  return TCL_OK;
}

/* convert canvas coordinate to time */
static int Pix2TimeCmd( tl, pix )
timeLineInfo *tl;
double pix;
{
  sprintf( tl->interp->result, "%.17g", Pix2Time( tl, pix ) );

  return TCL_OK;
}



/* change to a different visible portion of the canvas */
static int Xview( tl, i )
timeLineInfo *tl;
int i;
{
  char cmd[TMP_CMD_LEN];

  sprintf( cmd, "%s xview %d", tl->canvasName, i );
  if (TCL_OK != Tcl_Eval( tl->interp, cmd ))
    fprintf( stderr, "%s\n", tl->interp->result );
  tl->visLeft = tl->farLeft + 10 * i;
  ResetBounds( tl );
    /* add any object that may now be in view */
  UpdateVisibleRegion( tl, UPDATE_NEW_VIS );

#if DEBUG>1
  fprintf( stderr, "left time: %f, right time: %f\n", tl->leftTime,
	   tl->rightTime );
#endif

  return TCL_OK;
}


/*
   Change to a different visible portion of the canvas, given
   the time that is to be at the far left.
*/

static int ViewFromTime( tl, time )
timeLineInfo *tl;
double time;
{
  char cmd[TMP_CMD_LEN];
  int coord;

  coord = Time2Pix( tl, time );

    /* round down to multiple of 10 */
  coord -= ((coord - tl->farLeft) % 10);
  time = Pix2Time( tl, (double)coord );

  sprintf( cmd, "%s xview %d", tl->canvasName, coord );
  if (TCL_OK != Tcl_Eval( tl->interp, cmd ))
    fprintf( stderr, "%s\n", tl->interp->result );
  tl->visLeft = coord;

    /* reset the records that help determine whether an object
       is visible or not */
  ResetBounds( tl );
    /* add any object that may now be in view */
  UpdateVisibleRegion( tl, UPDATE_NEW_VIS );

#if DEBUG>1
  fprintf( stderr, "left time: %f, right time: %f\n", tl->leftTime,
	   tl->rightTime );
#endif

  return TCL_OK;
}


/*
   Change to a different visible portion of the canvas, given
   the time that is to be at the far left.
*/

static int ViewTime( tl, from_time, to_time )
timeLineInfo *tl;
double from_time, to_time;
{
  double zoom_factor;

  if (TCL_OK != ViewFromTime( tl, from_time ))
    return TCL_ERROR;

  zoom_factor = (tl->rightTime - tl->leftTime) / (to_time - from_time);
  if (TCL_OK != ZoomTime( tl, from_time, zoom_factor ))
    return TCL_ERROR;

  return TCL_OK;
}



#if 0

static int CanvasSize( canvas, width, height )
char *canvas;
int *width, *height;
{
  char cmd[200];
  int result;

    /* call [winfo width .c] and [winfo height .c] */
  sprintf( cmd, "winfo width %s", canvas );
  if (TCL_OK != (result = Tcl_Eval( interp, cmd ))) return result;
  *width = atoi( interp->result );

  sprintf( cmd, "winfo height %s", canvas );
  if (TCL_OK != (result = Tcl_Eval( interp, cmd ))) return result;
  *height = atoi( interp->result );

#if DEBUG
  fprintf( stderr, "width and height of %s: %d, %d\n",
	   canvas, *width, *height );
#endif

  return 0;
}

#endif



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



static double Time2Pix( tl, time )
timeLineInfo *tl;
double time;
{
  double pix, startTime;

  startTime = Log_StartTime( tl->log );

  pix = (time - startTime) /
    (Log_EndTime(tl->log) - startTime) *
      (tl->farRight - tl->farLeft) +
	tl->farLeft;
#if DEBUG>1
  fprintf( stderr, "Convert time %f to pixel %f\n", time, pix );
#endif

  return pix;
}

static double Pix2Time( tl, pix )
timeLineInfo *tl;
double pix;
{
  double time, startTime;

  startTime = Log_StartTime( tl->log );

  time = (pix - tl->farLeft) /
    (tl->farRight - tl->farLeft) *
      (Log_EndTime( tl->log ) - startTime) +
	startTime;
#if DEBUG>1
  fprintf( stderr, "Convert pixel %f to time %f\n", pix, time );
#endif

  return time;
}



  /* given an absolute process number, return the vertical position
     of the start of process timeline are.  Note that the exact
     timeline for, say process 2, exists at 2.5.  Value undefined
     if the process is not visible */
static double Proc2Pix( tl, proc )
timeLineInfo *tl;
double proc;
{
  double pix;

    /* convert visible process number to height within given display */
  pix = proc / Vis_n( tl->procVis ) *
    (tl->farBottom - tl->farTop) +
      tl->farTop;
#if DEBUG>1
  fprintf( stderr, "Convert process %f to pixel %f\n", proc, pix );
#endif

  return pix;
}

    /* convert pixel y-val to a process number (may be fractional
       if in the middle of a process region) */
static double Pix2Proc( tl, pix )
timeLineInfo *tl;
double pix;
{
  double time;
  time = (pix - tl->farTop) /
    (tl->farBottom - tl->farTop) * Vis_n( tl->procVis );
#if DEBUG>1
  fprintf( stderr, "Convert pixel %f to process %f\n", pix, time );
#endif
  return time;
}


static int ResetBounds( tl )
timeLineInfo *tl;
{
  tl->topProc = Pix2Proc( tl, (double)tl->visTop );
  tl->bottomProc = Pix2Proc( tl, (double)(tl->visTop + tl->visHeight) );
  tl->leftTime = Pix2Time( tl, (double)tl->visLeft );
  tl->rightTime = Pix2Time( tl, (double)(tl->visLeft + tl->visWidth) );

#if DEBUG>1
  fprintf( stderr, "boundaries: (%f, %f) - (%f, %f)\n", 
	   tl->leftTime, tl->topProc, tl->rightTime,
	   tl->bottomProc );
 fprintf( stderr, "or: (%d, %d) - (%d, %d)\n",
	   tl->visLeft, tl->visTop,
	   tl->visLeft + tl->visWidth,
	   tl->visTop + tl->visHeight );
#endif

  return 0;
}



static int StateOnScreen( tl, type, proc, startTime, endTime )
timeLineInfo *tl;
int type, proc;
double startTime, endTime;
{
  /* return 0 if the state is definitely not visible, 1 if it
     may be partially visible */
  int actualProc;		/* convert virtual process number to it's */
				/* index in the list of VISIBLE ones */

  actualProc = Vis_x2i( tl->procVis, proc );

#if DEBUG_VIS>1
  fprintf( stderr, "Is %d (or %d) on (%f,%f) within (%f,%f)-(%f,%f)? ",
	   proc, actualProc, startTime, endTime,
	   tl->leftTime, tl->topProc, tl->rightTime,
	   tl->bottomProc+1 );
#endif

  if (startTime > tl->rightTime ||
      endTime < tl->leftTime ||
      actualProc < (int)tl->topProc ||
      actualProc > (int)tl->bottomProc+1) {
#if DEBUG_VIS>1
    fprintf( stderr, "No.\n" );
#endif
    return 0;
  } else {
#if DEBUG_VIS>1
    fprintf( stderr, "Yes.\n" );
#endif
    return 1;
  }
}



static int TimeLineDrawState( data_v, idx, type, proc, startTime, endTime,
			      parent, firstChild, overlapLevel )
void *data_v;
int idx;
int type, proc, parent, firstChild, overlapLevel;
double startTime, endTime;
{
  static int update_count = 0;

  char *argv[20];
  char nums[4][20];		/* temp storage for stringized numbers */

  timeLineInfo *tl;
  double l, r, t, b;
  int visibleProcIdx, firstChildCanvasId;
    /* corners of the state: left, right, top, bottom */
  char cmd[1000];
  tl_stateInfo *state_info;
  char *state_name, *state_color, *state_bitmap;
  int i;

  Tcl_CmdInfo cmd_info;

#if DEBUG>1
  fprintf( stderr, "TimeLineDrawState sent state #%d.\n", idx );
  fprintf( stderr, "%f to %f, proc %d, type %d, overlapLevel %d\n",
	   startTime, endTime, proc,
	   type, overlapLevel );
#endif


  tl = (timeLineInfo *) data_v;

    /* if the list of state information for the timeline is not big
       enough for the given index, expand it */
  if (idx >= ListSize( tl->stateList, tl_stateInfo )) {
    state_info = (tl_stateInfo*) malloc( sizeof( tl_stateInfo ) );
    state_info->canvasId1 = TL_STATE_NOT_VISIBLE;
    for (i=ListSize( tl->stateList, tl_stateInfo ); i<idx+1; i++) {
      ListAddItem( tl->stateList, tl_stateInfo, *state_info );
    }
    free( state_info );
  }
      
  state_info = ListHeadPtr( tl->stateList, tl_stateInfo ) + idx;

  if (StateOnScreen( tl, type, proc, startTime, endTime )) {

    Log_GetStateDef( tl->log, type, &state_name, &state_color,
		     &state_bitmap );

#if DEBUG_VIS
    fprintf( stderr, "It's visible\n" );
#endif

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

    if (tl->bw) {
      argv[7] = "-fill";
      argv[8] = tl->bg;
      argv[9] = "-outline";
      argv[10] = tl->bg;
      argv[11] = 0;
      
/*
      sprintf( cmd,
               "%s create rect %.17g %.17g %.17g %.17g -fill %s -outline %s",
	       tl->canvasName, l, t, r, b, tl->bg, tl->bg );
#if DEBUG_TK_DRAWS
      fprintf( stderr, "%s\n", cmd );
#endif
      if (TCL_OK != Tcl_Eval( tl->interp, cmd )) {
	fprintf( stderr, "%s\n", tl->interp->result );
	return TCL_ERROR;
      }
*/

        /* call the canvas command manually */
      TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 11, argv );

        /* get canvas id */
      sscanf( tl->interp->result, "%d", &state_info->canvasId1 );

      argv[7] = "-fill";
      argv[8] = tl->outlineColor;
      argv[9] = "-outline";
      argv[10] = tl->outlineColor;
      argv[11] = "-stipple";
      argv[12] = state_bitmap;
      argv[13] = 0;

/*
      sprintf( cmd, "%s create rect %.17g %.17g %.17g %.17g -outline %s -fill %s -stipple %s",
	       tl->canvasName, l, t, r, b, tl->outlineColor, tl->outlineColor,
	       state_bitmap );
#if DEBUG_TK_DRAWS
      fprintf( stderr, "%s\n", cmd );
#endif
      if (TCL_OK != Tcl_Eval( tl->interp, cmd )) {
	fprintf( stderr, "%s\n", tl->interp->result );
	return TCL_ERROR;
      }
*/


        /* call the canvas command manually */
      TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 13, argv );

        /* get canvas id */
      sscanf( tl->interp->result, "%d", &state_info->canvasId2 );

        /* if this state has children, drop it below their rectangles */
      if (firstChildCanvasId != -1) {

	sprintf( nums[0], "%d", state_info->canvasId2 );
	sprintf( nums[1], "%d", firstChildCanvasId );
	argv[1] = "lower";
	argv[2] = nums[0];
	argv[3] = nums[1];
	argv[4] = 0;

/*
	sprintf( cmd, "%s lower %d %d", tl->canvasName,
		 state_info->canvasId2, firstChildCanvasId );
	if (TCL_OK != Tcl_Eval( tl->interp, cmd ))
	  fprintf( stderr, "%s\n", tl->interp->result );
#if DEBUG_TK_DRAWS
	fprintf( stderr, "%s : %s\n", cmd, tl->interp->result );
#endif
*/

          /* drop this rectangle below its first child */
	TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 4, argv );

	sprintf( nums[1], "%d", state_info->canvasId1 );
	argv[2] = nums[1];
	argv[3] = nums[0];	/* still canvasId2 */

          /* drop this rectangle below its first child */
	TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 4, argv );
  
#if 0
	  /* drop the background below the stippled rect */
	sprintf( cmd, "%s lower %d %d", tl->canvasName, state_info->canvasId1,
		 state_info->canvasId2 );
	if (TCL_OK != Tcl_Eval( tl->interp, cmd ))
	  fprintf( stderr, "%s\n", tl->interp->result );
#if DEBUG_TK_DRAWS
	fprintf( stderr, "%s : %s\n", cmd, tl->interp->result );
#endif
#endif

      }

    } else {


      /* color state */

#if 0
      sprintf( cmd,
	       "%s create rect %.17g %.17g %.17g %.17g -outline %s -fill %s",
	       tl->canvasName, l, t, r, b, tl->outlineColor,
	       state_color );
#if DEBUG_TK_DRAWS
      fprintf( stderr, "%s\n", cmd );
#endif
      if (TCL_OK != Tcl_Eval( tl->interp, cmd )) {
	fprintf( stderr, "%s\n", tl->interp->result );
	return TCL_ERROR;
      }
#endif

      argv[7] = "-fill";
      argv[8] = state_color;
      argv[9] = "-outline";
      argv[10] = tl->outlineColor;
      argv[11] = 0;
      
        /* drop this rectangle below its first child */
      TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 11, argv );

        /* get canvas id */
      sscanf( tl->interp->result, "%d", &state_info->canvasId1 );
        /* copy into canvasId2 so that canvasId2 can always be checked
	   against [.c find withtag current] */
      state_info->canvasId2 = state_info->canvasId1;

        /* if this state has children, drop this below them */
      if (firstChildCanvasId != -1) {
	
#if 0
          /* drop this rectangle below its first child */
	sprintf( cmd, "%s lower %d %d", tl->canvasName, state_info->canvasId1,
		 firstChildCanvasId );
	if (TCL_OK != Tcl_Eval( tl->interp, cmd ))
	  fprintf( stderr, "%s\n", tl->interp->result );
#if DEBUG_TK_DRAWS
	fprintf( stderr, "%s :%s\n", cmd, tl->interp->result );
#endif
#endif

	sprintf( nums[0], "%d", state_info->canvasId1 );
	sprintf( nums[1], "%d", firstChildCanvasId );
	argv[1] = "lower";
	argv[2] = nums[0];
	argv[3] = nums[1];
	argv[4] = 0;

          /* drop this rectangle below its first child */
	TRY_TCL_CMD_ARGV( cmd_info, tl->interp, 4, argv );
      }
    }

  } else {
    /* if the state is not visible */
    state_info->canvasId1 = TL_STATE_NOT_VISIBLE;
  }

    /* just to keep things moving */
    /* leave out TK_IDLE_EVENTS since we want to give everything a chance
       except the canvas redraw */
  while (Tk_DoOneEvent( TK_DONT_WAIT | TK_X_EVENTS | TK_FILE_EVENTS |
		        TK_TIMER_EVENTS )) {
#if DEBUG
    fprintf( stderr, "do one event\n" );
#endif
  }


  update_count++;
  if (update_count == UPDATES_EVERY_N_STATES) {
    strcpy( cmd, "update" );
    Tcl_Eval( tl->interp, cmd );
#if DEBUG
    fprintf( stderr, "update\n" );
#endif
    update_count = 0;
  }

  return 0;
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

int CurrentItem( tl )
timeLineInfo *tl;
{
  int i, n;
  int canvas_id;
  char cmd[TMP_CMD_LEN];
  tl_stateInfo *statePtr;

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



static int UpdateVisibleRegion( tl, flags )
timeLineInfo *tl;
int flags;
{
  int *list1, n1, *list2, n2;

  if (flags & UPDATE_NEW_VIS) {

      /* get two ranges of state indices that may have become visible */
      /* first one is sorted by start time, second by end time */
      /* no good reason for having two lists except it was easier to
	 implement */
    Log_GetVisStates( tl->log, tl->leftTime, tl->rightTime,
		      &list1, &n1, &list2, &n2 );

    UpdateStateList( tl, list1, n1 );
    UpdateStateList( tl, list2, n2 );
  }

  return 0;
}
