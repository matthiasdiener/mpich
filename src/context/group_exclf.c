/* group_excl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

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
void mpi_group_excl_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint * ));

void mpi_group_excl_ ( group, n, ranks, newgroup, __ierr )
MPI_Fint *group; 
MPI_Fint *n; 
MPI_Fint *ranks;
MPI_Fint *newgroup;
MPI_Fint *__ierr;
{
    MPI_Group l_newgroup;
   
    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_excl( MPI_Group_f2c(*group), *n, ranks, 
                                  &l_newgroup );
    else {
	int *l_ranks;
	int i;

	MPIR_FALLOC(l_ranks,(int*)MALLOC(sizeof(int)* (int)*n),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_excl");
	for (i=0; i<*n; i++)
	    l_ranks[i] = (int)ranks[i];
	
        *__ierr = MPI_Group_excl( MPI_Group_f2c(*group), (int)*n, l_ranks, 
                                  &l_newgroup );

	FREE( l_ranks );
    }
    *newgroup = MPI_Group_c2f(l_newgroup);
}

