/*
   time_lbl.h - time label widget for Upshot

   Ed Karrels
   Argonne National Laboratory

   This will implement a widget that will display scrolling time labels
to match a display whose horizontal axis is time.  Labels will only appear
at integer powers of 10, or simple fractions thereof.  If Im saying this
correctly or not, I mean that labels will be at 1.001, 1.002, 1.003,
or 1.00100, 1.00125, 1.00150, 1.00175, but not 1.0017865, 1.0027865, etc.
The precision displayed will be adjusted to what is appropriate.
As many labels as reasonable possible will be fit on the display.


   time_lbl <window name> <logfile token>
     window name - name of the window to create and the widget command
     logfile token - token for the logfile the label is to be attached to

   <window name> set <totalUnits> <windowUnits> <firstUnit> <lastUnit>
     same as a scrollbar [set] command

*/


#ifndef _TIME_LBL_H_
#define _TIME_LBL_H_

#include "tcl.h"
#include "tk.h"
#include "log.h"
#include "log_widget.h"

#ifdef __STDC__
int time_lblAppInit( Tcl_Interp *interp );
#else
int time_lblAppInit();
#endif

/* structure defining an individual hashmark, to be used in a linked
   list of hash marks */
typedef struct timelbl_hashmark_ {
    /* indices that make up this object on the canvas */
  int text_canvas_idx, hashMark_canvas_idx;
    /* which label this is, if the leftmost label is 0 */
  int lbl_idx;
    /* pointer to the next lbl in the list */
  struct timelbl_hashmark_ *next;
} timelbl_hashmark;

typedef struct timelbl_ {
  Tcl_Interp *interp;
  Tk_Window win;
  char *windowName, *canvasName;
    /* fg - color for the text and hashes
       bg - background color 
       font - font for the text */
  logData *log;
  char *fg, *bg, *font;
    /* sprintf format string used to print all labels */
  char *format_str;
    /* pulled from the logfile */
  double starttime, endtime;
    /* window width, from the latest resize event */
  double visWidth;
    /* total width, from the latest resize event combined with */
    /* latest 'scroll set' */
  double width;
    /* total height of the widget, based on text size */
  double height;
    /* how much space between label centers */
  double lbl_width;
    /* x-coordinate of leftmost label */
  double lbl_startx;
    /* the settings from the latest 'scroll set' command */
    /* certain recalculations and redraws will only be necessary if these
       numbers change */
  int scroll_total, scroll_visible;

  double firstHashTime;		/* value for leftmost hash mark */
  double hashIncrement;		/* increment between hash marks */

  timelbl_hashmark *head, *tail;
    /* list of indices of which labels are currently visible */
    /* The indices used are figured as if each visible label is indexed, */
    /* and the leftmost label is 0 */
} timelbl;

/*
   Tk_ConfigSpec's:

   # win - TK_CONFIG_WINDOW - Tk_Window # maybe not

   fg - TK_CONFIG_COLOR - XColor *
   bg - TK_CONFIG_COLOR - XColor *
   font - TK_CONFIG_FONT - XFontStruct *
   width - TK_CONFIG_PIXELS - int
*/



#endif
