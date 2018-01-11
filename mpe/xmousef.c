/* xmouse.c */
/* Fortran interface file for sun4 */
#include <stdio.h>
#include "mpe.h"
extern void *MPIR_ToPointer(); extern int MPIR_FromPointer();
#include "tools.h"
#include "basex11.h"
#include "mpe.h"

 mpe_get_mouse_press_( graph, x, y, button, __ierr )
MPE_XGraph*graph;
int *x, *y, *button;
int *__ierr;
{
*__ierr = MPE_Get_mouse_press(*graph,x,y,button);
}
 void mpe_iget_mouse_press_( graph, x, y, button, wasPressed, __ierr )
MPE_XGraph*graph;
int *x, *y, *button, *wasPressed;
int *__ierr;
{
*__ierr = MPE_Iget_mouse_press(*graph,x,y,button,wasPressed);
}
