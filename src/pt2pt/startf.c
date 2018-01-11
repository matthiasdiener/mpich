/* start.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_start_ PMPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ pmpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ pmpi_start
#else
#define mpi_start_ pmpi_start_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_start_ MPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ mpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ mpi_start
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_start_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_start_( request, __ierr )
MPI_Fint *request;
MPI_Fint *__ierr;
{
    MPI_Request lrequest = MPI_Request_f2c(*request );
    *__ierr = MPI_Start( &lrequest );
    *request = MPI_Request_c2f(lrequest);
}
