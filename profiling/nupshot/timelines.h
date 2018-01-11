/*
   Timeline widget for Upshot

   Ed Karrels
   Argonne National Laboratory

   This widget should handle all the operation of a timeline canvas,
with everything controlled not in pixels or boxes or arrows, but
in seconds, states, and messages.

Operations that will be allowed:

  Create and connect to a logfile

    timeline <window name> <logfile token> <total width> <total height> \
             <req width> <req height> [options...]
      width - total width, not just the visible width, in one of the
          forms acceptable to Tk_GetPixels
      height - total height, not just the visible height, in one of the
          forms acceptable to Tk_GetPixels

      options:
        -from <time>
        -to <time>
            Specify the earliest point in the logfile that will
            be displayed.  If the logfile is huge, the user may only
            wish to load part of it.  Say, in a 1 hour logfile, the
            user may only want to load from the 15 minute mark to the
            20 minute mark.  It will be possible to reconfigure the
            canvas to dispay a different region.

    Every timeline widget must be connected to a logfile widget that
    has been opened, but not necessarily loaded.  The timeline will get
    all of its settings such as the number of processes and the
    display option for the states from the logfile to which it is
    connected.

    Also, the timeline widget will grab a hook (any number of display
    widgets can grab this hook, BTW) so that the timeline widget
    will be told about all states, events, and message that
    are visited by the logfile widget.  So, as the logfile is loaded,
    possible incrementally, the definitions of each state, event, or
    message instance will be sent to the timeline widget so that it
    may draw them.  Or not, if they are out of view, or the process
    on which the instance occurs is not in view right now.

  Add objects
    It will be possible to add events, states, or message to
    the canvas manually.  This is low priority; all adding should
    be done at the C level, but we might as well be flexible.

  Scrolling
    Like a canvas:
      <timeline> scroll scan mark <x> <y>
      <timeline> scroll scan dragto <x> <y>
      <timeline> scroll xview <x>
      <timeline> scroll yview <y>
      <timeline> config -xscrollcommand <command>
      <timeline> config -yscrollcommand <command>

    The -xscrollcommand option will be bound to the logfile that
    this timeline to which this logfile is connected.  The scan
    mark, scan dragto, and xview commands (changing x-coordinate only)
    will be called by the same logfile.

    In the future, the y-scrolling commands may be controlled, too.

  Zooming
    zoom by a given factor ( f>1 for zoom in, 0<f<1 for zoom out) at
    a given point

      <timeline> zoom_time <time> <factor>


  Eliminating or adding certain processes, events, states, or message
    There will be an option to display or not certain processes,
    events, states, or messages.

  Finish drawing all, cut down to visible region, delete out of view
    The default behavior will be to not draw objects that are not
    in the visible portion of the canvas until they are scrolled into
    view.  They will then be left on the canvas.  This is to cut down
    the initial lag time before the user actually sees anything
    interesting.

    There will be an option to finish drawing all the objects on the
    canvas, including the objects not in the visible
    portion of the canvas.  This will slow down scrolling since
    more objects will have to be manipulated by the canvas object,
    but it will speed it up since they won't be constantly drawn whenever
    they come into view.

    There will be an option to delete any item on the canvas that is not
    in the visible range.  This is in case a lot of objects have been
    drawn, slowing down the canvas.  Deleting unneeded objects should
    speed up scrolling and zooming.

    There will be a setting to specify whether or not to delete objects
    as they are scrolled out of view.  I'm not sure if this will speed
    up scrolling since less object will be dealt with or if it
    will slow it down since they will have to constantly be recreated.
    It will probably slow it down, since the create/deletion time
    of object probably greatly outweighs the time to move it.
    Maybe this option is a bad idea.  Hmmm.

    Note that at no time should the user be able to scroll the timeline
    widget to a region where the objects have not been drawn yet.
    Before scrolling to a region, the timeline widget should be sure
    to draw any objects that may lay (at least partially) in the new
    range.

*/


#ifndef _TIMELINES_H_
#define _TIMELINES_H_

#include "log.h"
#include "vis.h"
#include "expandingList.h"

#define TL_STATE_NOT_VISIBLE -1

  /* bit mask of what needs to be done */
  /* in the next TimeLineWhenIdle() */
#define TL_NEEDS_RESIZE (1<<0)

#define UPDATE_NEW_VIS (1<<0)


typedef struct tl_stateInfo_ {
  int canvasId1, canvasId2;
} tl_stateInfo;
  /* if canvasId1 >= 0, it is the index of the item id of the rectangle
     on the canvas.  If the display is B&W, canvasId1 is the index
     of this guys matching coverup rectangle, and canvasId2 is the id
     of the stippled rectangle.

     if canvasId1 < 0:
     TL_STATE_NOT_VISIBLE - state is not in view; was not drawn
  */

typedef struct timeLineOverlap_ {
  xpandList /*int*/ halfWidths;
} timeLineOverlap;


typedef struct timeLineConvert_ {
  double offset;
  double skew;
} timeLineConvert;

typedef struct timeLineInfo_ {
  Tcl_Interp *interp;		/* Tcl interpreter */
  logData *log;			/* the log its attached to */
  char *windowName;		/* the window for this widget, and its */
				/* widget command */
  Tk_Window win;		/* the real window definition */
  char *canvasName;		/* the display canvas's name */
  int width, height;		/* total width & height */

  int farLeft, farRight;	/* scroll region */
  int farTop, farBottom;

  double visTime;		/* total amount time that is visible in one */
				/* screen width; default is all */

  double visProcs;		/* total number of process visible at a */
				/* time; default is all */

  int visWidth, visHeight;	/* visible width & height */
  int visLeft, visTop;		/* left and right visible coordinate */

  double topProc, leftTime;	/* uppermost visible process and leftmost
                                   visible time--shows scroll position */
  double bottomProc, rightTime;	/* other end of the visible range */
				/* these four fields provide a quick */
				/* method of checking if an object is */
				/* visible and are computed from */
				/* far{Top,Bottom,Left,Right} and */
				/* vis{Top,Left,Width,Height} */

  Vis *procVis;			/* visible processes */
  Vis *eventVis;		/* visible events */
  Vis *stateVis;		/* visible states */
  Vis *msgVis;			/* visible messages */
  timeLineConvert cvt_time;	/* convert time to horiz */
  timeLineConvert cvt_proc;	/* convert process # to vert */
				/* (center of timeline is .5) */
  timeLineOverlap overlap;	/* overlap info */
  char *outlineColor;		/* color of outlines for state bars */
  char *lineColor;		/* timeline color */
  char *msgColor;		/* color of message arrows */
  char *bg;			/* canvas background color */
  char *bitmapdir;		/* directory of bitmaps */
  char *xscrollCommand;		/* copy to canvas -xscrollcommand */
  char *yscrollCommand;		/* copy to canvas -yscrollcommand */
  int bw;			/* in black&white */
  int is_bw;			/* for switch "-bw" */
  int is_color;			/* for switch "-color" */

  double xresizeFactor;		/* factor by which the canvas needs to */
  double yresizeFactor;		/* be resized (at the next update) */

  int whenIdle;			/* bit mask of what needs to be done */
				/* in the next TimeLineWhenIdle() */

  void *drawStateToken;		/* token returned by Log_AddDrawState(), */
				/* must be returned to Log_RmDrawState() */

  xpandList /*tl_stateInfo*/ stateList;
    /* list of all states, noting if each was drawn, and if so, what
       its index is.  This list is parallel to the stateData->list list
       of states, so the same index will be valid for both */

} timeLineInfo;


#endif  /* _TIMELINES_H_ */
