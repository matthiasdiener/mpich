/* ic_merge.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ PMPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge
#else
#define mpi_intercomm_merge_ pmpi_intercomm_merge_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ MPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_intercomm_merge_ ANSI_ARGS(( MPI_Comm *, int *, MPI_Comm *, int * ));

void mpi_intercomm_merge_ ( comm, high, comm_out, __ierr )
MPI_Comm  *comm;
int*high;
MPI_Comm *comm_out;
int *__ierr;
{
    *__ierr = MPI_Intercomm_merge( *comm, *high, comm_out );
}
