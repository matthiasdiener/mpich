/* group_compare.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_compare_ PMPI_GROUP_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_compare_ pmpi_group_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_compare_ pmpi_group_compare
#else
#define mpi_group_compare_ pmpi_group_compare_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_compare_ MPI_GROUP_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_compare_ mpi_group_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_compare_ mpi_group_compare
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_compare_ ANSI_ARGS(( MPI_Group *, MPI_Group *, int *, int * ));

void mpi_group_compare_ ( group1, group2, result, __ierr )
MPI_Group  *group1;
MPI_Group  *group2;
int       *result;
int *__ierr;
{
    *__ierr = MPI_Group_compare( *group1, *group2, result );
}
