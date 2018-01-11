/* probe.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_probe_ PMPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ pmpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ pmpi_probe
#else
#define mpi_probe_ pmpi_probe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_probe_ MPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ mpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ mpi_probe
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_probe_ ANSI_ARGS(( int *, int *, MPI_Comm *, MPI_Status *, int * ));

void mpi_probe_( source, tag, comm, status, __ierr )
int*source;
int*tag;
MPI_Comm    *comm;
MPI_Status  *status;
int *__ierr;
{
    *__ierr = MPI_Probe(*source,*tag,*comm,status);
}
