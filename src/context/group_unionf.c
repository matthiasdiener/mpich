/* group_union.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_union_ PMPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ pmpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ pmpi_group_union
#else
#define mpi_group_union_ pmpi_group_union_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_union_ MPI_GROUP_UNION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_union_ mpi_group_union__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_union_ mpi_group_union
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_union_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				  MPI_Fint * ));

void mpi_group_union_ ( group1, group2, group_out, __ierr )
MPI_Fint *group1; 
MPI_Fint *group2; 
MPI_Fint *group_out;
MPI_Fint *__ierr;
{
    MPI_Group l_group_out;
    *__ierr = MPI_Group_union( MPI_Group_f2c(*group1), 
                               MPI_Group_f2c(*group2), 
                               &l_group_out );
    *group_out = l_group_out;
}
