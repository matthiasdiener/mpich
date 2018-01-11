/* timers.c */
/* Fortran interface file for sun4 */
#include <stdio.h>
#include "mpe.h"
double mpe_ptime_ ANSI_ARGS((void));
 double  mpe_ptime_()
{
return MPE_Ptime();
}
double mpe_wtime_ ANSI_ARGS((void));

 double  mpe_wtime_()
{
return MPE_Wtime();
}
