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
void mpi_group_compare_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                    MPI_Fint * ));

void mpi_group_compare_ ( group1, group2, result, __ierr )
MPI_Fint *group1;
MPI_Fint *group2;
MPI_Fint *result;
MPI_Fint *__ierr;
{
    int l_result;
    *__ierr = MPI_Group_compare( MPI_Group_f2c(*group1), 
                                 MPI_Group_f2c(*group2), &l_result );
    *result = l_result;
}
