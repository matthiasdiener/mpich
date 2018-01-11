/* group_tranks.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_translate_ranks_ PMPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks
#else
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_translate_ranks_ MPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_translate_ranks_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint * ));
void mpi_group_translate_ranks_ ( group_a, n, ranks_a, group_b, ranks_b,
				  __ierr )
MPI_Fint *group_a;
MPI_Fint *n;
MPI_Fint *ranks_a;
MPI_Fint *group_b;
MPI_Fint *ranks_b;
MPI_Fint *__ierr;
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_translate_ranks(MPI_Group_f2c(*group_a),*n,
                                            ranks_a,
                                            MPI_Group_f2c(*group_b), 
                                            ranks_b);
    else {
        int *l_ranks_a;
        int *l_ranks_b;
        int i;

	MPIR_FALLOC(l_ranks_a,(int*)MALLOC(sizeof(int)* (int)*n),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_translate_ranks");

	MPIR_FALLOC(l_ranks_b,(int*)MALLOC(sizeof(int)* *(n)),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_translate_ranks");

        for (i=0; i<(int)*n; i++) {
            l_ranks_a[i] = (int)ranks_a[i];
    } 
        *__ierr = MPI_Group_translate_ranks(MPI_Group_f2c(*group_a),(int)*n,
                                            l_ranks_a,
                                            MPI_Group_f2c(*group_b), 
                                            l_ranks_b);
        for (i=0; i<(int)*n; i++) {
            ranks_b[i] = (MPI_Fint)l_ranks_b;
        }
	FREE( l_ranks_a );
	FREE( l_ranks_b );
    }
    
}
