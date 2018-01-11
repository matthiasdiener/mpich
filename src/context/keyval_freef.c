/* keyval_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_keyval_free_ PMPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free
#else
#define mpi_keyval_free_ pmpi_keyval_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_keyval_free_ MPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_keyval_free_ ANSI_ARGS(( int *, int * ));

void mpi_keyval_free_ ( keyval, __ierr )
int *keyval;
int *__ierr;
{
    *__ierr = MPI_Keyval_free(keyval);
}
