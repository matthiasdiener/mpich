/* mpe_graphics.c */
/* Fortran interface file for sun4 */
#include <stdio.h>
#include "mpe.h"
extern void *MPIR_ToPointer(); extern int MPIR_FromPointer();
#include "tools.h"
#include "basex11.h"
#include "mpe.h"
 void mpe_open_graphics_( handle, comm, display, x, y, w, h, is_collective, __ierr )
MPE_XGraph *handle;
MPI_Comm   comm;
char       display[];
int*x,*y;
int*w,*h;
int*is_collective;
int *__ierr;
{
*__ierr = MPE_Open_graphics(
	(MPE_XGraph* )MPIR_ToPointer( *(int*)(handle) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),display,*x,*y,*w,*h,*is_collective);
}
 void mpe_draw_point_( handle, x, y, color, __ierr )
MPE_XGraph*handle;
int*x,*y;
MPE_Color*color;
int *__ierr;
{
*__ierr = MPE_Draw_point(*handle,*x,*y,*color);
}
 void mpe_draw_points_( handle, points, npoints, __ierr )
MPE_XGraph*handle;
MPE_Point *points;
int*npoints;
int *__ierr;
{
*__ierr = MPE_Draw_points(*handle,
	(MPE_Point* )MPIR_ToPointer( *(int*)(points) ),*npoints);
}
 void mpe_draw_line_( handle, x1, y1, x2, y2, color, __ierr )
MPE_XGraph*handle;
int*x1,*y1,*x2,*y2;
MPE_Color*color;
int *__ierr;
{
*__ierr = MPE_Draw_line(*handle,*x1,*y1,*x2,*y2,*color);
}
 void mpe_fill_rectangle_( handle, x, y, w, h, color, __ierr )
MPE_XGraph*handle;
int*x,*y,*w,*h;
MPE_Color*color;
int *__ierr;
{
*__ierr = MPE_Fill_rectangle(*handle,*x,*y,*w,*h,*color);
}
 void mpe_update_( handle, __ierr )
MPE_XGraph*handle;
int *__ierr;
{
*__ierr = MPE_Update(*handle);
}
 void mpe_close_graphics_( handle, __ierr )
MPE_XGraph *handle;
int *__ierr;
{
*__ierr = MPE_Close_graphics(
	(MPE_XGraph* )MPIR_ToPointer( *(int*)(handle) ));
}
 void mpe_make_color_array_( handle, ncolors, array, __ierr )
MPE_XGraph*handle;
int*ncolors;
MPE_Color  array[];
int *__ierr;
{
*__ierr = MPE_Make_color_array(*handle,*ncolors,
	(MPE_Color* )MPIR_ToPointer( *(int*)(array) ));
}
 void mpe_num_colors_( handle, nc, __ierr )
MPE_XGraph*handle;
int        *nc;
int *__ierr;
{
*__ierr = MPE_Num_colors(*handle,nc);
}
 void mpe_draw_circle_( graph, centerx, centery, radius, color, __ierr )
MPE_XGraph*graph;
int*centerx,*centery,*radius;
MPE_Color*color;
int *__ierr;
{
*__ierr = MPE_Draw_circle(*graph,*centerx,*centery,*radius,*color);
}
 void mpe_draw_logic_( graph, function, __ierr )
MPE_XGraph*graph;
int*function;
int *__ierr;
{
*__ierr = MPE_Draw_logic(*graph,*function);
}
 void mpe_line_thickness_( graph, thickness, __ierr )
MPE_XGraph*graph;
int*thickness;
int *__ierr;
{
*__ierr = MPE_Line_thickness(*graph,*thickness);
}
 void mpe_add_rgb_color_( graph, red, green, blue, mapping, __ierr )
MPE_XGraph*graph;
int*red,*green,*blue;
MPE_Color *mapping;
int *__ierr;
{
*__ierr = MPE_Add_RGB_color(*graph,*red,*green,*blue,
	(MPE_Color* )MPIR_ToPointer( *(int*)(mapping) ));
}
