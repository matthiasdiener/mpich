/* $Id: basex11.h,v 1.1 1994/06/29 20:58:44 karrels Exp $ */


/*
    This file contains a basic X11 data structure that may be used within
    other structures for basic graphics operations.
 */

#if !defined(_BASEX11) && !defined(TOOLSNOX11)
#define _BASEX11

/* AIX assumes that sys/types is included before Xutil is (when it defines
   function prototypes) */
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef unsigned long PixVal;

/* Our rule on GC is that the current pixel value is remembered so that
   we don't contsantly call a routine to change it when it is already the
   value that we want. */
typedef struct {
    GC       set;
    PixVal   cur_pix;
    } GCCache;
   
/* 
   Many routines need the display, window, and a GC; 
   occasionally, routines need the visual and the colormap (particularly
   those doing scientific imaging).  For scaling information, the
   region of the window is also needed (note that many XBWindow
   structures can use the same Window) 
 */
typedef struct {
    Display  *disp;
    int      screen;
    Window   win;
    GCCache  gc;
    Visual   *vis;            /* Graphics visual */
    int      depth;           /* Depth of visual */
    int      numcolors,       /* Number of available colors */
             maxcolors;       /* Current number in use */
    Colormap cmap;
    PixVal   foreground, background;
    PixVal   cmapping[256];
    int      x, y, w, h;      /* Size and location of window */
    /* The following permit double buffering; by making this part of the
       XBWindow structure, everyone can utilize double buffering without
       any special arrangements.  If buf is not null, all routines draw 
       to it instead, and XBFlush does a copyarea. NOT YET IMPLEMENTED */
    Drawable drw;
    } XBWindow;

/* This definition picks the drawable to use for an X operation.  This
   should be used for all drawing routines (note that some routines need
   a Window, not just a drawable). */
#define XBDrawable(w) ((w)->drw ? (w)->drw : (w)->win)

/* There are a number of properties that we'd like to have on hand about 
   a font; in particular, a bound on the size of a character */
typedef struct {
    Font     fnt;
    int      font_w, font_h;
    int      font_descent;
    PixVal   font_pix;
    } XBFont;

/* This is a user-defined coordinates region */
typedef struct {
    double  xmin,xmax,ymin,ymax,zmin,zmax ;
    } XBAppRegion;

typedef struct {
    int      x, y, xh, yh, w, h;
    } XBRegion;

/* This is the "decoration" structure.  This could later involve
   patterns to be used outside the frame, as well as a "background"
   (interior) decoration */
typedef struct {
    XBRegion Box;
    int      width, HasColor, is_in;
    PixVal   Hi, Lo;
    } XBDecoration;
    
#define XBSetPixVal( xbwin, pixval ) \
if (xbwin->gc.cur_pix != pixval) { \
    XSetForeground( xbwin->disp, xbwin->gc.set, pixval ); \
    xbwin->gc.cur_pix   = pixval;\
    }

/* Error returns */
#define ERR_CAN_NOT_OPEN_DISPLAY 0x10001
#define ERR_NO_DISPLAY           0x10002
#define ERR_CAN_NOT_OPEN_WINDOW  0x10003
#define ERR_ILLEGAL_SIZE         0x10004

/* Routines */
extern XBWindow *XBQGetWindow(), *XBWinCreate();
extern XBWindow *XBQExchangeQuickWindow();
extern void      XBWinDestroy();
extern int       XBQuickWindow();
extern XBFont   *XBFontFixed();
extern PixVal    XBGetColor();

#endif
