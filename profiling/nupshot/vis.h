/*
   Object visibility abstraction for Upshot

   Ed Karrels
   Argonne National Laboratory
*/

#ifndef _VIS_H_
#define _VIS_H_

#include "expandingList.h"

typedef struct Vis_ {
  xpandList /*int*/ list;
} Vis;


#ifdef __STDC__
Vis *Vis_new();
  /* create new visibility list */
int Vis_close( Vis * );
  /* close visibility list */
#if 0
Vis *Vis_all();
  /* create visibility list so that everything is visible (fast) */
  /* note - this is a special case.  Nothing may be added, removed,
     or read */
#endif
int Vis_add( Vis *, int x, int before );
  /* add x to visible list, before the given index (-1 for end) */ 
  /* may function as a move also--if the x is in the list
     it will be moved */
int Vis_rm( Vis *, int x );
  /* remove x from visible list, returns -1 if not found */
int Vis_n( Vis * );
  /* return the number of visible items */
int IsVis( Vis *, int x );
  /* return nonzero if x is visible */
int Vis_x2i( Vis *, int x );
  /* return the index of x in the visibility list */
int Vis_i2x( Vis *, int idx );
  /* return the item at the specified index in the visibility list */

#else

Vis *Vis_new();
int Vis_close();
int Vis_add();
int Vis_rm();
int Vis_n();
int IsVis();
int Vis_p2i();
int Vis_i2p();

#endif

#endif
