/* type_hind.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_hindexed_ PMPI_TYPE_HINDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hindexed_ pmpi_type_hindexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hindexed_ pmpi_type_hindexed
#else
#define mpi_type_hindexed_ pmpi_type_hindexed_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_hindexed_ MPI_TYPE_HINDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hindexed_ mpi_type_hindexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hindexed_ mpi_type_hindexed
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_hindexed_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint [], 
                                    MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_hindexed_( count, blocklens, indices, old_type, newtype, __ierr )
MPI_Fint *count;
MPI_Fint blocklens[];
MPI_Fint indices[];
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{
    MPI_Aint     *c_indices;
    MPI_Aint     local_c_indices[MPIR_USE_LOCAL_ARRAY];
    int          i;
    int          *l_blocklens; 
    int          local_l_blocklens[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype ldatatype;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	/* We really only need to do this when 
	   sizeof(MPI_Aint) != sizeof(INTEGER) */
	    MPIR_FALLOC(c_indices,(MPI_Aint *) MALLOC( *count * sizeof(MPI_Aint) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_TYPE_HINDEXED" );

	    MPIR_FALLOC(l_blocklens,(int *) MALLOC( *count * sizeof(int) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_TYPE_HINDEXED" );
	}
	else {
	    c_indices = local_c_indices;
	    l_blocklens = local_l_blocklens;
	}

	for (i=0; i<(int)*count; i++) {
	    c_indices[i] = (MPI_Aint) indices[i];
            l_blocklens[i] = (int) blocklens[i];
	}
	*__ierr = MPI_Type_hindexed((int)*count,l_blocklens,c_indices,
                                    MPI_Type_f2c(*old_type),
				    &ldatatype);
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    FREE( c_indices );
            FREE( l_blocklens );
	}
        *newtype = MPI_Type_c2f(ldatatype);
    }
    else if ((int)*count == 0) {
	*__ierr = MPI_SUCCESS;
        *newtype = 0;
    }
    else {
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_COUNT,
			      "Negative count in MPI_TYPE_HINDEXED" );
    }
}


