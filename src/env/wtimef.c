/* wtime.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_wtime_ PMPI_WTIME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtime_ pmpi_wtime__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtime_ pmpi_wtime
#else
#define mpi_wtime_ pmpi_wtime_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_wtime_ MPI_WTIME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtime_ mpi_wtime__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtime_ mpi_wtime
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
double mpi_wtime_ ANSI_ARGS(( void ));

double  mpi_wtime_()
{
    return MPI_Wtime();
}
