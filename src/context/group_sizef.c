/* group_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_size_ PMPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ pmpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ pmpi_group_size
#else
#define mpi_group_size_ pmpi_group_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_size_ MPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ mpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ mpi_group_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_group_size_ ( group, size, __ierr )
MPI_Fint *group;
MPI_Fint *size;
MPI_Fint *__ierr;
{
    int l_size;
    *__ierr = MPI_Group_size( MPI_Group_f2c(*group), &l_size );
    *size = l_size;
}
