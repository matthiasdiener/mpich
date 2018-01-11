/* comm_create.c */
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
#define mpi_comm_create_ PMPI_COMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_create_ pmpi_comm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_create_ pmpi_comm_create
#else
#define mpi_comm_create_ pmpi_comm_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_create_ MPI_COMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_create_ mpi_comm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_create_ mpi_comm_create
#endif
#endif

 void mpi_comm_create_ ( comm, group, comm_out, __ierr )
MPI_Comm  comm;
MPI_Group group;
MPI_Comm *comm_out;
int *__ierr;
{
MPI_Comm lcomm;
*__ierr = MPI_Comm_create(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),
	(MPI_Group)MPIR_ToPointer( *(int*)(group) ),&lcomm);
*(int*)comm_out = MPIR_FromPointer(lcomm);
}
