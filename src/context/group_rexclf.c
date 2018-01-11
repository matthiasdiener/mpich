/* group_rexcl.c */
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
#define mpi_group_range_excl_ PMPI_GROUP_RANGE_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_excl_ pmpi_group_range_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_excl_ pmpi_group_range_excl
#else
#define mpi_group_range_excl_ pmpi_group_range_excl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_range_excl_ MPI_GROUP_RANGE_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_excl_ mpi_group_range_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_excl_ mpi_group_range_excl
#endif
#endif

/* See the comments in group_rinclf.c.  ranges is correct without changes */
void mpi_group_range_excl_ ( group, n, ranges, newgroup, __ierr )
MPI_Group group, *newgroup;
int       *n;
int       ranges[][3];
int *__ierr;
{
MPI_Group lgroup;
*__ierr = MPI_Group_range_excl(
	(MPI_Group)MPIR_ToPointer(*((int*)group)),*n,ranges,&lgroup);
*(int*)newgroup = MPIR_FromPointer(lgroup);
}
