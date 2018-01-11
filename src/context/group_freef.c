/* group_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_free_ PMPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ pmpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ pmpi_group_free
#else
#define mpi_group_free_ pmpi_group_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_free_ MPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ mpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ mpi_group_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_group_free_ ( group, __ierr )
MPI_Fint *group;
MPI_Fint *__ierr;
{
    MPI_Group l_group = MPI_Group_f2c(*group);
    *__ierr = MPI_Group_free(&l_group);
    *group = MPI_Group_c2f(l_group);
}


