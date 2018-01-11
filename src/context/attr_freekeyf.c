/* attr_freekey.c */
/* Fortran interface file for sun4 */
#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif
#include "mpiimpl.h"
 void mpi_keyval_free_ ( keyval, __ierr )
int *keyval;
int *__ierr;
{
*__ierr = MPI_Keyval_free(keyval);
}
