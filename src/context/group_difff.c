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
void mpi_group_difference_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint * ));

void mpi_group_difference_ ( group1, group2, group_out, __ierr )
MPI_Fint *group1; 
MPI_Fint *group2; 
MPI_Fint *group_out;
MPI_Fint *__ierr;
{
    MPI_Group l_group_out;

    *__ierr = MPI_Group_difference(MPI_Group_f2c(*group1), 
                                   MPI_Group_f2c(*group2), 
                                   &l_group_out );
    *group_out = MPI_Group_c2f(l_group_out);
}
