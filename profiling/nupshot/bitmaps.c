/*

   A bunch of bitmaps

   2x2, black, boxes, dimple3, dllines3, dllines4, dlines3, drlines4,
   drlines4, gray, light_gray, gray3, hlines2, hlines3, hlines4,
   vlines2, vlines3, vlines4, white, zoom_in_horiz, zoom_out_horiz,
   zoom_in_vert, zoom_out_vert

*/

#include "tcl.h"
#include "tk.h"
#include "bitmaps.h"

static int bitmap_no = 0;
static char *bitmap_list[] = {
  "2x2", "black", "boxes", "dimple3", "dllines3",
  "dllines4", "dlines3", "drlines4", "drlines4", "gray", "light_gray",
  "gray3", "hlines2", "hlines3", "hlines4", "vlines2", "vlines3", "vlines4",
  "white", 0
};

int Bitmap_Reset() {
  bitmap_no = 0;
  return 0;
}

char *Bitmap_Get() {
  char *name;

  name = bitmap_list[bitmap_no++];
  if (!bitmap_list[bitmap_no])
    bitmap_no = 0;

  return name;
}

/* Some versions (all?) of tk.h use char, not unsigned char, for the bit 
   sources.  A compiler might complain about a mismatch 

   Unfortunately, 0xff causes OTHER compilers to complain about the
   value being out-of-range, since 0xff == 255 > mas value for a signed
   char (this happened on solaris).

   The only realistic fix is to define the data as unsigned, but cast the 
   values to signed.  
*/


#define x2x2_width 4
#define x2x2_height 4
static unsigned char x2x2_bits[] = {
   0x0f, 0x0f, 0x03, 0x03};


#define black_width 16
#define black_height 16
static unsigned char black_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


#define boxes_width 8
#define boxes_height 8
static unsigned char boxes_bits[] = {
   0x0f, 0x09, 0x09, 0x0f, 0xf0, 0x90, 0x90, 0xf0};


#define dimple3_width 16
#define dimple3_height 16
static unsigned char dimple3_bits[] = {
   0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define dllines3_width 9
#define dllines3_height 9
static unsigned char dllines3_bits[] = {
   0x24, 0x01, 0x92, 0x00, 0x49, 0x00, 0x24, 0x01, 0x92, 0x00, 0x49, 0x00,
   0x24, 0x01, 0x92, 0x00, 0x49, 0x00};


#define dllines4_width 4
#define dllines4_height 4
static unsigned char dllines4_bits[] = {
   0x01, 0x08, 0x04, 0x02};


#define dlines3_width 9
#define dlines3_height 9
static unsigned char dlines3_bits[] = {
   0x49, 0x00, 0x92, 0x00, 0x24, 0x01, 0x49, 0x00, 0x92, 0x00, 0x24, 0x01,
   0x49, 0x00, 0x92, 0x00, 0x24, 0x01};


#define drlines4_width 4
#define drlines4_height 4
static unsigned char drlines4_bits[] = {
   0x01, 0x02, 0x04, 0x08};


#define gray_width 2
#define gray_height 2
static unsigned char gray_bits[] = {
   0x01, 0x02};


#define light_gray_width 4
#define light_gray_height 2
static unsigned char light_gray_bits[] = {
   0x08, 0x02};


#define gray3_width 4
#define gray3_height 4
static unsigned char gray3_bits[] = {
   0x01, 0x00, 0x04, 0x00};


#define hlines2_width 16
#define hlines2_height 16
static unsigned char hlines2_bits[] = {
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00};


#define hlines3_width 15
#define hlines3_height 15
static unsigned char hlines3_bits[] = {
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0xff, 0x7f, 0x00, 0x00, 0x00, 0x00};


#define hlines4_width 16
#define hlines4_height 16
static unsigned char hlines4_bits[] = {
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define vlines2_width 4
#define vlines2_height 4
static unsigned char vlines2_bits[] = {
   0x0a, 0x0a, 0x0a, 0x0a};


#define vlines3_width 3
#define vlines3_height 1
static unsigned char vlines3_bits[] = {
   0x02};


#define vlines4_width 4
#define vlines4_height 4
static unsigned char vlines4_bits[] = {
   0x08, 0x08, 0x08, 0x08};


#define white_width 2
#define white_height 2
static unsigned char white_bits[] = {
   0x00, 0x00};

#define zoom_in_horiz_width 20
#define zoom_in_horiz_height 20
static unsigned char zoom_in_horiz_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x00, 0x60, 0x80, 0x01,
   0x70, 0x80, 0x03, 0xf8, 0xf3, 0x07, 0xfc, 0xf3, 0x0f, 0xfc, 0xf3, 0x0f,
   0xf8, 0xf3, 0x07, 0x70, 0x80, 0x03, 0x60, 0x80, 0x01, 0x40, 0x80, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define zoom_out_horiz_width 20
#define zoom_out_horiz_height 20
static unsigned char zoom_out_horiz_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x60, 0x80, 0x01,
   0xe0, 0xc0, 0x01, 0xfc, 0xe1, 0x0f, 0xfc, 0xf3, 0x0f, 0xfc, 0xf3, 0x0f,
   0xfc, 0xe1, 0x0f, 0xe0, 0xc0, 0x01, 0x60, 0x80, 0x01, 0x20, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define zoom_in_vert_width 20
#define zoom_in_vert_height 20
static unsigned char zoom_in_vert_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x3f, 0x00, 0x80, 0x7f, 0x00, 0xc0, 0xff, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0xc0, 0xff, 0x00,
   0x80, 0x7f, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x0c, 0x00};

#define zoom_out_vert_width 20
#define zoom_out_vert_height 20
static unsigned char zoom_out_vert_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x1e, 0x00, 0xc0, 0xff, 0x00, 0x80, 0x7f, 0x00, 0x00, 0x3f, 0x00,
   0x00, 0x1e, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x0c, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3f, 0x00, 0x80, 0x7f, 0x00,
   0xc0, 0xff, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00};


/* Define UNSIGNED as <empty> or unsigned, depending on the prototype */
#define UNSIGNED

int Bitmap_Init( interp )
Tcl_Interp *interp;
{
  Tk_DefineBitmap( interp, Tk_GetUid( "2x2" ), (UNSIGNED char *)x2x2_bits,
		   x2x2_width, x2x2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "black" ), (UNSIGNED char *)black_bits,
		   black_width, black_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "boxes" ), (UNSIGNED char *)boxes_bits,
		   boxes_width, boxes_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dimple3" ), 
		   (UNSIGNED char *)dimple3_bits,
		   dimple3_width, dimple3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dllines3" ), 
		   (UNSIGNED char *)dllines3_bits,
		   dllines3_width, dllines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dllines4" ), 
		   (UNSIGNED char *)dllines4_bits,
		   dllines4_width, dllines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "dlines3" ), 
		   (UNSIGNED char *)dlines3_bits,
		   dlines3_width, dlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "drlines4" ), 
		   (UNSIGNED char *)drlines4_bits,
		   drlines4_width, drlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "drlines4" ), 
		   (UNSIGNED char *)drlines4_bits,
		   drlines4_width, drlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "gray" ), (UNSIGNED char *)gray_bits,
		   gray_width, gray_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "light_gray" ), 
		   (UNSIGNED char *)light_gray_bits,
		   light_gray_width, light_gray_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "gray3" ), (UNSIGNED char *)gray3_bits,
		   gray3_width, gray3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines2" ), 
		   (UNSIGNED char *)hlines2_bits,
		   hlines2_width, hlines2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines3" ), 
		   (UNSIGNED char *)hlines3_bits,
		   hlines3_width, hlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "hlines4" ), 
		   (UNSIGNED char *)hlines4_bits,
		   hlines4_width, hlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines2" ), 
		   (UNSIGNED char *)vlines2_bits,
		   vlines2_width, vlines2_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines3" ), 
		   (UNSIGNED char *)vlines3_bits,
		   vlines3_width, vlines3_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "vlines4" ), 
		   (UNSIGNED char *)vlines4_bits,
		   vlines4_width, vlines4_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "white" ), (UNSIGNED char *)white_bits,
		   white_width, white_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "zoom_in_horiz" ), 
		   (UNSIGNED char *)zoom_in_horiz_bits,
		   zoom_in_horiz_width, zoom_in_horiz_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "zoom_out_horiz" ), 
		   (UNSIGNED char *)zoom_out_horiz_bits,
		   zoom_out_horiz_width, zoom_out_horiz_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "zoom_in_vert" ), 
		   (UNSIGNED char *)zoom_in_vert_bits,
		   zoom_in_vert_width, zoom_in_vert_height );
  Tk_DefineBitmap( interp, Tk_GetUid( "zoom_out_vert" ), 
		   (UNSIGNED char *)zoom_out_vert_bits,
		   zoom_out_vert_width, zoom_out_vert_height );

  return 0;
}
