/*

   Define a bunch of bitmaps

   2x2, black, boxes, dimple3, dllines3, dllines4, dlines3, drlines4,
   drlines4, gray, light_gray, gray3, hlines2, hlines3, hlines4,
   vlines2, vlines3, vlines4, white

*/

#ifndef BITMAPS_H
#define BITMAPS_H

#include "tcl.h"


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

int Bitmap_Init ARGS(( Tcl_Interp *interp ));
int Bitmap_Reset();
char *Bitmap_Get();


#endif
