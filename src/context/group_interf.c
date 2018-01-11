/* group_inter.c */
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
#define mpi_group_intersection_ PMPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection
#else
#define mpi_group_intersection_ pmpi_group_intersection_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_intersection_ MPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection
#endif
#endif

 void mpi_group_intersection_ ( group1, group2, group_out, __ierr )
MPI_Group group1, group2, *group_out;
int *__ierr;
{
MPI_Group lgroup;
*__ierr = MPI_Group_intersection(
	(MPI_Group)MPIR_ToPointer( *(int*)(group1) ),
	(MPI_Group)MPIR_ToPointer( *(int*)(group2) ), &lgroup);
*(int*)group_out = MPIR_FromPointer(lgroup);
}
