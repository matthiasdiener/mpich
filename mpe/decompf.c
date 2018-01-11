/* decomp.c */
/* Fortran interface file for sun4 */
#include <stdio.h>
#include "mpe.h"
extern void *MPIR_ToPointer(); extern int MPIR_FromPointer();
 void mpe_decomp1d_( n, size, rank, s, e, __ierr )
int*n,*size,*rank, *s, *e;
int *__ierr;
{
*__ierr = MPE_Decomp1d(*n,*size,*rank,s,e);
}
