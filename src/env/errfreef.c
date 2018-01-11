/* errfree.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_errhandler_free_ ANSI_ARGS(( MPI_Errhandler *, int * ));

void mpi_errhandler_free_( errhandler, __ierr )
MPI_Errhandler *errhandler;
int *__ierr;
{
    *__ierr = MPI_Errhandler_free( errhandler );
}
