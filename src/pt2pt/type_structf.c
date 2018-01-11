/* type_struct.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_struct_ PMPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct
#else
#define mpi_type_struct_ pmpi_type_struct_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_struct_ MPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_struct_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint [], 
                                  MPI_Fint [], MPI_Fint *, 
                                  MPI_Fint * ));

void mpi_type_struct_( count, blocklens, indices, old_types, newtype, __ierr )
MPI_Fint *count;
MPI_Fint blocklens[];
MPI_Fint indices[];      
MPI_Fint old_types[];
MPI_Fint *newtype;
MPI_Fint     *__ierr;
{
    MPI_Aint     *c_indices;
    MPI_Aint     local_c_indices[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype *l_datatype;
    MPI_Datatype local_l_datatype[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype l_newtype;
    int          *l_blocklens;
    int          local_l_blocklens[MPIR_USE_LOCAL_ARRAY];
    int          i;
    
    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	/* Since indices come from MPI_ADDRESS (the FORTRAN VERSION),
	   they are currently relative to MPIF_F_MPI_BOTTOM.  
	   Convert them back */
	    MPIR_FALLOC(c_indices,(MPI_Aint *) MALLOC( *count * sizeof(MPI_Aint) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPI_TYPE_STRUCT" );

	    MPIR_FALLOC(l_blocklens,(int *) MALLOC( *count * sizeof(int) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPI_TYPE_STRUCT" );

	    MPIR_FALLOC(l_datatype,(MPI_Datatype *) MALLOC( *count * sizeof(MPI_Datatype) ),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPI_TYPE_STRUCT" );
	}
	else {
	    c_indices = local_c_indices;
	    l_blocklens = local_l_blocklens;
	    l_datatype = local_l_datatype;
	}

	for (i=0; i<(int)*count; i++) {
	    c_indices[i] = (MPI_Aint) indices[i]/* + (MPI_Aint)MPIR_F_MPI_BOTTOM*/;
            l_blocklens[i] = (int) blocklens[i];
            l_datatype[i] = MPI_Type_f2c(old_types[i]);
	}
	*__ierr = MPI_Type_struct((int)*count, l_blocklens, c_indices,
                                  l_datatype,
				  &l_newtype);

        if ((int)*count > MPIR_USE_LOCAL_ARRAY) {    
	    FREE( c_indices );
            FREE( l_blocklens );
            FREE( l_datatype );
	}
    }
    else if ((int)*count == 0) {
	*__ierr = MPI_SUCCESS;
	*newtype = 0;
    }
    else {
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_COUNT,
			      "Negative count in MPI_TYPE_STRUCT" );
    }
    *newtype = MPI_Type_c2f(l_newtype);

}
