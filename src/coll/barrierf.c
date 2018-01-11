/* barrier.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_barrier_ PMPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ pmpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ pmpi_barrier
#else
#define mpi_barrier_ pmpi_barrier_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_barrier_ MPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ mpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ mpi_barrier
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_barrier_ ANSI_ARGS(( MPI_Comm *, int * ));

void mpi_barrier_ ( comm, __ierr )
MPI_Comm *comm;
int *__ierr;
{
    *__ierr = MPI_Barrier( *comm );
}
