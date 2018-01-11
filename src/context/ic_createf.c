/* ic_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

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

 void mpi_intercomm_create_ ( local_comm, local_leader, peer_comm, 
                           remote_leader, tag, comm_out, __ierr )
MPI_Comm  local_comm;
int*local_leader;
MPI_Comm  peer_comm;
int*remote_leader;
int*tag;
MPI_Comm *comm_out;
int *__ierr;
{
MPI_Comm lcomm;
*__ierr = MPI_Intercomm_create(
	(MPI_Comm)MPIR_ToPointer( *(int*)(local_comm) ),*local_leader,
	(MPI_Comm)MPIR_ToPointer( *(int*)(peer_comm) ),*remote_leader,*tag,
			       &lcomm );
*(int*)comm_out = MPIR_FromPointer(lcomm);
}
