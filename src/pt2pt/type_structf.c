/* type_struct.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

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
void mpi_type_struct_ ANSI_ARGS(( int *, int [], int [], MPI_Datatype [],
				  MPI_Datatype *, int * ));

void mpi_type_struct_( count, blocklens, indices, old_types, newtype, __ierr )
int           *count;
int           blocklens[];
int           indices[];      
MPI_Datatype  old_types[];
MPI_Datatype  *newtype;
int           *__ierr;
{
    MPI_Aint     *c_indices;
    int          i;

    if (*count > 0) {
	/* Since indices come from MPI_ADDRESS (the FORTRAN VERSION),
	   they are currently relative to MPIF_F_MPI_BOTTOM.  
	   Convert them back */
	MPIR_FALLOC(c_indices,(MPI_Aint *) MALLOC( *count * sizeof(MPI_Aint) ),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPI_TYPE_STRUCT" );

	for (i=0; i<*count; i++) {
	    c_indices[i] = (MPI_Aint) indices[i]/* + (MPI_Aint)MPIR_F_MPI_BOTTOM*/;
	}
	*__ierr = MPI_Type_struct(*count,blocklens,c_indices,old_types,
				  newtype);
    
	FREE( c_indices );
    }
    else if (*count == 0) {
	*__ierr = MPI_SUCCESS;
	*newtype = 0;
    }
    else {
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_COUNT,
			      "Negative count in MPI_TYPE_STRUCT" );
    }

}
