/* comm_group.c */
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
#define mpi_comm_group_ PMPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_group_ pmpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_group_ pmpi_comm_group
#else
#define mpi_comm_group_ pmpi_comm_group_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_group_ MPI_COMM_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_group_ mpi_comm_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_group_ mpi_comm_group
#endif
#endif

 void mpi_comm_group_ ( comm, group, __ierr )
MPI_Comm comm;
MPI_Group *group;
int *__ierr;
{
MPI_Group lgroup;
*__ierr = MPI_Comm_group(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ), &lgroup);
*(int*)group = MPIR_FromPointer(lgroup);
}
