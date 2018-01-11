/* group_union.c */
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
#define mpi_group_union_ PMPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ pmpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ pmpi_group_union
#else
#define mpi_group_union_ pmpi_group_union_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_union_ MPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ mpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ mpi_group_union
#endif
#endif

 void mpi_group_union_ ( group1, group2, group_out, __ierr )
MPI_Group group1, group2, *group_out;
int *__ierr;
{
MPI_Group lgroup;
*__ierr = MPI_Group_union(
	(MPI_Group)MPIR_ToPointer( *(int*)(group1) ),
	(MPI_Group)MPIR_ToPointer( *(int*)(group2) ), &lgroup);
*(int*)group_out = MPIR_FromPointer(lgroup);
}
