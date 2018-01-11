/* group_inter.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_intersection_ PMPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ pmpi_group_intersection
#else
#define mpi_group_intersection_ pmpi_group_intersection_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_intersection_ MPI_GROUP_INTERSECTION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_intersection_ mpi_group_intersection
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_intersection_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                         MPI_Fint *,
					 MPI_Fint * ));

void mpi_group_intersection_ ( group1, group2, group_out, __ierr )
MPI_Fint *group1; 
MPI_Fint *group2; 
MPI_Fint *group_out;
MPI_Fint *__ierr;
{
    MPI_Group l_group_out;
    *__ierr = MPI_Group_intersection(MPI_Group_f2c(*group1), 
                                     MPI_Group_f2c(*group2), 
                                     &l_group_out );
    *group_out = MPI_Group_c2f(l_group_out);
}
