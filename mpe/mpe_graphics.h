#ifndef _MPE_GRAPHICS_
#define _MPE_GRAPHICS_

#ifdef MPE_NOMPI
typedef int MPI_Comm;
#else
#include "mpi.h"
#endif

typedef enum { MPE_WHITE = 0, MPE_BLACK = 1, MPE_RED = 2, MPE_YELLOW = 3, 
	       MPE_GREEN = 4, MPE_CYAN = 5, MPE_BLUE = 6 } MPE_Color;

extern int MPE_buttonArray[];
extern int MPE_logicArray[];

  /* given existing pixel 'dst' and new, overlapping pixel 'src' */
#define MPE_LOGIC_CLEAR        (MPE_logicArray[0])
#define MPE_LOGIC_AND          (MPE_logicArray[1])
#define MPE_LOGIC_ANDREVERSE   (MPE_logicArray[2])
#define MPE_LOGIC_COPY         (MPE_logicArray[3])
#define MPE_LOGIC_ANDINVERTED  (MPE_logicArray[4])
#define MPE_LOGIC_NOOP         (MPE_logicArray[5])
#define MPE_LOGIC_XOR          (MPE_logicArray[6])
#define MPE_LOGIC_OR           (MPE_logicArray[7])
#define MPE_LOGIC_NOR          (MPE_logicArray[8])
#define MPE_LOGIC_EQUIV        (MPE_logicArray[9])
#define MPE_LOGIC_INVERT       (MPE_logicArray[10])
#define MPE_LOGIC_ORREVERSE    (MPE_logicArray[11])
#define MPE_LOGIC_COPYINVERTED (MPE_logicArray[12])
#define MPE_LOGIC_ORINVERTED   (MPE_logicArray[13])
#define MPE_LOGIC_NAND         (MPE_logicArray[14])
#define MPE_LOGIC_SET          (MPE_logicArray[15])

#define MPE_BUTTON1 (MPE_buttonArray[0])
#define MPE_BUTTON2 (MPE_buttonArray[1])
#define MPE_BUTTON3 (MPE_buttonArray[2])
#define MPE_BUTTON4 (MPE_buttonArray[3])
#define MPE_BUTTON5 (MPE_buttonArray[4])


/* types of visuals for Get_drag_region */
#define MPE_DRAG_NONE 0		     /* no visual */
#define MPE_DRAG_RECT 1		     /* rubber band box */
#define MPE_DRAG_LINE 2		     /* rubber band line */
#define MPE_DRAG_CIRCLE_RADIUS 3     /* rubber band circle, */
    /* one point is the center of the circle, other point is on the circle */
#define MPE_DRAG_CIRCLE_DIAMETER 4
    /* each point is on opposite sides of the circle */
#define MPE_DRAG_CIRCLE_BBOX 5
    /* the two points define a bounding box inside which is drawn a circle */
#define MPE_DRAG_OVAL_BBOX 6
    /* the two points define a bounding box inside which is drawn an oval */
#define MPE_DRAG_SQUARE 7


#ifdef MPE_INTERNAL
#include "tools.h"
#include "basex11.h"

struct MPE_XGraph_s {
  int      Cookie;
  XBWindow *xwin;
  int      backingStore;	/* NotUseful, WhenMapped, or Always */
  MPI_Comm comm;
  int      is_collective;
  char     *display_name;       /* Used to allow us to run other tools ... */
  char     *capture_file;       /* Used to capture output at update */
  int      capture_num, 
           capture_cnt, 
           capture_freq;
};
typedef struct MPE_XGraph_s *MPE_XGraph;
#define MPE_G_COOKIE 0xfeeddada

#define MPE_XEVT_IDLE_MASK 0
/* normal XEvent mask; what it should be set to during normal processing */
/* Eventually, this should be ExposureMask or more */

#else

typedef void *MPE_XGraph;

#endif

typedef struct MPE_Point_ {
  int x, y;
  MPE_Color c;
} MPE_Point;

#define MPE_GRAPH_INDEPDENT  0
#define MPE_GRAPH_COLLECTIVE 1

/* I got this trick from the Tcl implementation */
#ifdef _ANSI_ARGS_
#undef _ANSI_ARGS_
#endif

#ifdef __STDC__
#define _ANSI_ARGS_(x) x
#else
#define _ANSI_ARGS_(x) ()
#endif

extern int MPE_Open_graphics _ANSI_ARGS_(( MPE_XGraph *handle, MPI_Comm comm, 
	   char *display, int x, int y, int w, int h, int is_collective ));

extern int MPE_Get_mouse_press _ANSI_ARGS_(( MPE_XGraph graph,
           int *x, int *y, int *button ));

extern int MPE_Get_drag_region _ANSI_ARGS_(( MPE_XGraph graph, int button,
           int dragVisual, int *pressx, int *pressy, int *releasex,
	   int *releasey ));

extern int MPE_Draw_point _ANSI_ARGS_(( MPE_XGraph handle, int x, int y,
	   MPE_Color color ));

extern int MPE_Draw_line _ANSI_ARGS_(( MPE_XGraph handle, int x1, int y1,
	   int x2, int y2, MPE_Color color ));

extern int MPE_Draw_circle _ANSI_ARGS_(( MPE_XGraph handle, int centerx,
	   int centery, int radius, MPE_Color color ));

extern int MPE_Fill_rectangle _ANSI_ARGS_(( MPE_XGraph handle, int x, int y,
	   int w, int h, MPE_Color color ));

extern int MPE_Update _ANSI_ARGS_(( MPE_XGraph handle ));

extern int MPE_Num_colors _ANSI_ARGS_(( MPE_XGraph handle, int *nc ));

extern int MPE_Make_color_array _ANSI_ARGS_(( MPE_XGraph handle, int ncolors, 
	   MPE_Color array[] ));

extern int MPE_Close_graphics _ANSI_ARGS_(( MPE_XGraph *handle ));

#endif

