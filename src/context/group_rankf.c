/* group_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_rank_ ANSI_ARGS(( MPI_Group *, int *, int * ));

void mpi_group_rank_ ( group, rank, __ierr )
MPI_Group *group;
int *rank;
int *__ierr;
{
    *__ierr = MPI_Group_rank( *group, rank );
}
