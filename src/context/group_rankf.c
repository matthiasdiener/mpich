/* group_rank.c */
/* Fortran interface file */
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
#define mpi_group_rank_ PMPI_GROUP_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_rank_ pmpi_group_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_rank_ pmpi_group_rank
#else
#define mpi_group_rank_ pmpi_group_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_rank_ MPI_GROUP_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_rank_ mpi_group_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_rank_ mpi_group_rank
#endif
#endif

 void mpi_group_rank_ ( group, rank, __ierr )
MPI_Group group;
int *rank;
int *__ierr;
{
*__ierr = MPI_Group_rank(
	(MPI_Group)MPIR_ToPointer( *(int*)(group) ),rank);
}
