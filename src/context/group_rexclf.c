/* group_rexcl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_range_excl_ ANSI_ARGS(( MPI_Group *, int *, int [][3], 
				       MPI_Group *, int * ));

/* See the comments in group_rinclf.c.  ranges is correct without changes */
void mpi_group_range_excl_ ( group, n, ranges, newgroup, __ierr )
MPI_Group *group, *newgroup;
int       *n;
int       ranges[][3];
int *__ierr;
{
    *__ierr = MPI_Group_range_excl(*group,*n,ranges,newgroup);
}
