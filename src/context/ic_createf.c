/* ic_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_intercomm_create_ PMPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_create_ pmpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_create_ pmpi_intercomm_create
#else
#define mpi_intercomm_create_ pmpi_intercomm_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_intercomm_create_ MPI_INTERCOMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_create_ mpi_intercomm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_create_ mpi_intercomm_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_intercomm_create_ ANSI_ARGS(( MPI_Comm *, int *, MPI_Comm *,
				       int *, int *, MPI_Comm *, int * ));

void mpi_intercomm_create_ ( local_comm, local_leader, peer_comm, 
                           remote_leader, tag, comm_out, __ierr )
MPI_Comm  *local_comm;
int*local_leader;
MPI_Comm  *peer_comm;
int*remote_leader;
int*tag;
MPI_Comm *comm_out;
int *__ierr;
{
    *__ierr = MPI_Intercomm_create( *local_comm, *local_leader,
				    *peer_comm, *remote_leader,*tag,
				    comm_out );
}
