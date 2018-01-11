/* keyval_free.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
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

 void mpi_keyval_free_ ( keyval, __ierr )
int *keyval;
int *__ierr;
{
*__ierr = MPI_Keyval_free(keyval);
}
