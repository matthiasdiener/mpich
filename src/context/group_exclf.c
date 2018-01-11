/* group_excl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_excl_ PMPI_GROUP_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_excl_ pmpi_group_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_excl_ pmpi_group_excl
#else
#define mpi_group_excl_ pmpi_group_excl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_excl_ MPI_GROUP_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_excl_ mpi_group_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_excl_ mpi_group_excl
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_excl_ ANSI_ARGS(( MPI_Group *, int *, int *, MPI_Group *, 
				 int * ));

void mpi_group_excl_ ( group, n, ranks, newgroup, __ierr )
MPI_Group *group, *newgroup;
int*n, *ranks;
int *__ierr;
{
    *__ierr = MPI_Group_excl( *group, *n,ranks, newgroup );
}
