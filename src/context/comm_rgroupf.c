/* comm_rgroup.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_remote_group_ PMPI_COMM_REMOTE_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_group_ pmpi_comm_remote_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_group_ pmpi_comm_remote_group
#else
#define mpi_comm_remote_group_ pmpi_comm_remote_group_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_remote_group_ MPI_COMM_REMOTE_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_group_ mpi_comm_remote_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_group_ mpi_comm_remote_group
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_remote_group_ ANSI_ARGS(( MPI_Comm, MPI_Group *, int * ));

void mpi_comm_remote_group_ ( comm, group, __ierr )
MPI_Comm comm;
MPI_Group *group;
int *__ierr;
{
    MPI_Group lgroup;
    *__ierr = MPI_Comm_remote_group(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ), &lgroup);
    *(int*)group = MPIR_FromPointer(lgroup);
}
