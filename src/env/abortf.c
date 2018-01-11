/* abort.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_abort_ PMPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_abort_ pmpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_abort_ pmpi_abort
#else
#define mpi_abort_ pmpi_abort_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_abort_ MPI_ABORT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_abort_ mpi_abort__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_abort_ mpi_abort
#endif
#endif
/* Prototype to suppress warnings about missing prototypes */
void mpi_abort_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_abort_( comm, errorcode, __ierr )
MPI_Fint *comm;
MPI_Fint *errorcode;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Abort( MPI_Comm_f2c(*comm), (int)*errorcode);
}
