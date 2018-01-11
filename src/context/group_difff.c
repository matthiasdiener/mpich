/* group_diff.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_difference_ PMPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_difference_ pmpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_difference_ pmpi_group_difference
#else
#define mpi_group_difference_ pmpi_group_difference_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_difference_ MPI_GROUP_DIFFERENCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_difference_ mpi_group_difference__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_difference_ mpi_group_difference
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_difference_ ANSI_ARGS(( MPI_Group *, MPI_Group *, MPI_Group *,
				       int * ));

void mpi_group_difference_ ( group1, group2, group_out, __ierr )
MPI_Group *group1, *group2, *group_out;
int *__ierr;
{
    *__ierr = MPI_Group_difference(*group1, *group2, group_out );
}
