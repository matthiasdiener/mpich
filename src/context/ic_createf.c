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
void mpi_intercomm_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint * ));

void mpi_intercomm_create_ ( local_comm, local_leader, peer_comm, 
                           remote_leader, tag, comm_out, __ierr )
MPI_Fint *local_comm;
MPI_Fint *local_leader;
MPI_Fint *peer_comm;
MPI_Fint *remote_leader;
MPI_Fint *tag;
MPI_Fint *comm_out;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm_out;
    *__ierr = MPI_Intercomm_create( MPI_Comm_f2c(*local_comm), 
                                    (int)*local_leader, 
                                    MPI_Comm_f2c(*peer_comm), 
                                    (int)*remote_leader, (int)*tag,
				    &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
