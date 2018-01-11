/* type_ind.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_indexed_ PMPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed
#else
#define mpi_type_indexed_ pmpi_type_indexed_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_indexed_ MPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_indexed_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint [], 
                                   MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_indexed_( count, blocklens, indices, old_type, newtype, __ierr )
MPI_Fint *count;
MPI_Fint blocklens[];
MPI_Fint indices[];
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{
    int          i;
    int          *l_blocklens;
    int          local_l_blocklens[MPIR_USE_LOCAL_ARRAY];
    int          *l_indices;
    int          local_l_indices[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype ldatatype;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(l_blocklens,(int *) MALLOC( *count * sizeof(int) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_TYPE_INDEXED" );

	    MPIR_FALLOC(l_indices,(int *) MALLOC( *count * sizeof(int) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_TYPE_INDEXED" );
	}
	else {
	    l_blocklens = local_l_blocklens;
	    l_indices = local_l_indices;
	}

        for (i=0; i<(int)*count; i++) {
	    l_indices[i] = (int)indices[i];
	    l_blocklens[i] = (int)blocklens[i];
         }
    }
 
    *__ierr = MPI_Type_indexed((int)*count, l_blocklens, l_indices,
                               MPI_Type_f2c(*old_type), 
                               &ldatatype);
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
        FREE( l_indices );
        FREE( l_blocklens );
    }
    *newtype = MPI_Type_c2f(ldatatype);
}




