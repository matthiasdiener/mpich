/*

   A bunch of bitmaps

   2x2, black, boxes, dimple3, dllines3, dllines4, dlines3, drlines4,
   drlines4, gray, light_gray, gray3, hlines2, hlines3, hlines4,
   vlines2, vlines3, vlines4, white

*/

#include "tcl.h"
#include "tk.h"



#define x2x2_width 4
#define x2x2_height 4
static char x2x2_bits[] = {
   0x0f, 0x0f, 0x03, 0x03};


#define black_width 16
#define black_height 16
static char black_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


#define boxes_width 8
#define boxes_height 8
static char boxes_bits[] = {
   0x0f, 0x09, 0x09, 0x0f, 0xf0, 0x90, 0x90, 0xf0};


#define dimple3_width 16
#define dimple3_height 16
static char dimple3_bits[] = {
   0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define dllines3_width 9
#define dllines3_height 9
static char dllines3_bits[] = {
   0x24, 0x01, 0x92, 0x00, 0x49, 0x00, 0x24, 0x01, 0x92, 0x00, 0x49, 0x00,
   0x24, 0x01, 0x92, 0x00, 0x49, 0x00};


#define dllines4_width 4
#define dllines4_height 4
static char dllines4_bits[] = {
   0x01, 0x08, 0x04, 0x02};


#define dlines3_width 9
#define dlines3_height 9
static char dlines3_bits[] = {
   0x49, 0x00, 0x92, 0x00, 0x24, 0x01, 0x49, 0x00, 0x92, 0x00, 0x24, 0x01,
   0x49, 0x00, 0x92, 0x00, 0x24, 0x01};


#define drlines4_width 4
#define drlines4_height 4
static char drlines4_bits[] = {
   0x01, 0x02, 0x04, 0x08};


#define gray_width 2
#define gray_height 2
static char gray_bits[] = {
   0x01, 0x02};


#define light_gray_width 4
#define light_gray_height 2
static char light_gray_bits[] = {
   0x08, 0x02};


#define gray3_width 4
#define gray3_height 4
static char gray3_bits[] = {
   0x01, 0x00, 0x04, 0x00};


#define hlines2_width 16
#define hlines2_height 16
static char hlines2_bits[] = {
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00};


#define hlines3_width 15
#define hlines3_height 15
static char hlines3_bits[] = {
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00};


#define hlines4_width 16
#define hlines4_height 16
static char hlines4_bits[] = {
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define vlines2_width 4
#define vlines2_height 4
static char vlines2_bits[] = {
   0x0a, 0x0a, 0x0a, 0x0a};


#define vlines3_width 3
#define vlines3_height 1
static char vlines3_bits[] = {
   0x02};


#define vlines4_width 4
#define vlines4_height 4
static char vlines4_bits[] = {
   0x08, 0x08, 0x08, 0x08};


#define white_width 2
#define white_height 2
static char white_bits[] = {
   0x00, 0x00};


int RegisterBitmaps( interp )
Tcl_Interp *interp;
{
  Tk_DefineBitmap( interp, Tk_GetUid( "2x2" ), x2x2_bits,
		   x2x2_width, x2x2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "black" ), black_bits,
		   black_width, black_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "boxes" ), boxes_bits,
		   boxes_width, boxes_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dimple3" ), dimple3_bits,
		   dimple3_width, dimple3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dllines3" ), dllines3_bits,
		   dllines3_width, dllines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dllines4" ), dllines4_bits,
		   dllines4_width, dllines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dlines3" ), dlines3_bits,
		   dlines3_width, dlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "drlines4" ), drlines4_bits,
		   drlines4_width, drlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "drlines4" ), drlines4_bits,
		   drlines4_width, drlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "gray" ), gray_bits,
		   gray_width, gray_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "light_gray" ), light_gray_bits,
		   light_gray_width, light_gray_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "gray3" ), gray3_bits,
		   gray3_width, gray3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines2" ), hlines2_bits,
		   hlines2_width, hlines2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines3" ), hlines3_bits,
		   hlines3_width, hlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines4" ), hlines4_bits,
		   hlines4_width, hlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines2" ), vlines2_bits,
		   vlines2_width, vlines2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines3" ), vlines3_bits,
		   vlines3_width, vlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines4" ), vlines4_bits,
		   vlines4_width, vlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "white" ), white_bits,
		   white_width, white_height );

  return 0;
}
