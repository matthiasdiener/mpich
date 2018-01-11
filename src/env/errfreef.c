/* errfree.c */
/* Custom Fortran interface file */
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
#define mpi_errhandler_free_ PMPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_free_ pmpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_free_ pmpi_errhandler_free
#else
#define mpi_errhandler_free_ pmpi_errhandler_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_free_ MPI_ERRHANDLER_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_free_ mpi_errhandler_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_free_ mpi_errhandler_free
#endif
#endif

void mpi_errhandler_free_( errhandler, __ierr )
MPI_Errhandler *errhandler;
int *__ierr;
{
MPI_Errhandler old;
old = (MPI_Errhandler)MPIR_ToPointer( *errhandler );
*__ierr = MPI_Errhandler_free( &old );
*(int *)errhandler = MPI_ERRHANDLER_NULL;
}
