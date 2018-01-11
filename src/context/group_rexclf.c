/* group_rexcl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_range_excl_ PMPI_GROUP_RANGE_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_excl_ pmpi_group_range_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_excl_ pmpi_group_range_excl
#else
#define mpi_group_range_excl_ pmpi_group_range_excl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_range_excl_ MPI_GROUP_RANGE_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_excl_ mpi_group_range_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_excl_ mpi_group_range_excl
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_range_excl_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint [][3], MPI_Fint *, 
                                       MPI_Fint * ));

/* See the comments in group_rinclf.c.  ranges is correct without changes */
void mpi_group_range_excl_ ( group, n, ranges, newgroup, __ierr )
MPI_Fint *group; 
MPI_Fint *n;
MPI_Fint ranges[][3];
MPI_Fint *newgroup;
MPI_Fint *__ierr;
{
    MPI_Group l_newgroup;

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_range_excl(MPI_Group_f2c(*group),*n,
                                       ranges, &l_newgroup);
    else {
	int *l_ranges;
	int i;
	int j = 0;

        MPIR_FALLOC(l_ranges,(int*)MALLOC(sizeof(int)* ((int)*n * 3)),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_range_excl");

        for (i=0; i<*n; i++) {
	    l_ranges[j++] = (int)ranges[i][0];
	    l_ranges[j++] = (int)ranges[i][1];
	    l_ranges[j++] = (int)ranges[i][2];
	}
	
        *__ierr = MPI_Group_range_excl(MPI_Group_f2c(*group), (int)*n,
                                       (int (*)[3])l_ranges, &l_newgroup);
	FREE( l_ranges );
	
    }
    *newgroup = MPI_Group_c2f(l_newgroup);
}
