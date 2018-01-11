/* group_incl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_incl_ PMPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl
#else
#define mpi_group_incl_ pmpi_group_incl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_incl_ MPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_incl_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint * ));

void mpi_group_incl_ ( group, n, ranks, group_out, __ierr )
MPI_Fint *group; 
MPI_Fint *n; 
MPI_Fint *ranks;
MPI_Fint *group_out;
MPI_Fint *__ierr;
{
    MPI_Group l_group_out;

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_incl( MPI_Group_f2c(*group), *n,
                                  ranks, &l_group_out );
    else {
	int *l_ranks;
	int i;

	MPIR_FALLOC(l_ranks,(int*)MALLOC(sizeof(int)* (int)*n),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_incl");
	for (i=0; i<*n; i++)
	    l_ranks[i] = (int)ranks[i];

        *__ierr = MPI_Group_incl( MPI_Group_f2c(*group), (int)*n,
                                  l_ranks, &l_group_out );
	
	FREE( l_ranks );
    }

    *group_out = MPI_Group_c2f(l_group_out);
}
