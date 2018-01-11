/* finalize.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_finalize_ PMPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ pmpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ pmpi_finalize
#else
#define mpi_finalize_ pmpi_finalize_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_finalize_ MPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ mpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ mpi_finalize
#endif
#endif
/* Prototype to suppress warnings about missing prototypes */
void mpi_finalize_ ANSI_ARGS(( int * ));

void mpi_finalize_(__ierr )
int *__ierr;
{
    *__ierr = MPI_Finalize();
}
