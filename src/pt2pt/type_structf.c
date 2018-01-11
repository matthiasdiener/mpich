/* type_struct.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
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
#ifdef POINTER_64_BITS
void mpi_type_struct_ ANSI_ARGS(( int *, int [], int [], int [],
				  MPI_Datatype *, int * ));
#else
void mpi_type_struct_ ANSI_ARGS(( int *, int [], int [], MPI_Datatype [],
				  MPI_Datatype *, int * ));
#endif
void mpi_type_struct_( count, blocklens, indices, old_types, newtype, __ierr )
int           *count;
int           blocklens[];
int           indices[];      
#ifdef POINTER_64_BITS
/* If old_types is an ARRAY of MPI_Datatype, then if pointers and Fortran 
   integers are different lengths, this code will be confused.
 */
int           old_types[];
#else
MPI_Datatype  old_types[];
#endif
MPI_Datatype  *newtype;
int           *__ierr;
{
    MPI_Datatype lnewtype = 0;
    MPI_Aint     *c_indices;
    int          i;
#ifdef POINTER_64_BITS
    MPI_Datatype *old;
#endif

    if (*count > 0) {
	/* We really only need to do this when 
	   sizeof(MPI_Aint) != sizeof(INTEGER) */
	MPIR_FALLOC(c_indices,(MPI_Aint *) MALLOC( *count * sizeof(MPI_Aint) ),
		    MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Out of space in MPI_TYPE_HINDEXED" );

	for (i=0; i<*count; i++) {
	    c_indices[i] = (MPI_Aint) indices[i];
	}
    }
    else if (*count == 0) {
	*__ierr = MPI_SUCCESS;
	*(int*)newtype = 0;
    }
    else {
	*__ierr = MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_COUNT,
			      "Negative count in MPI_TYPE_STRUCT" );
    }

#ifdef POINTER_64_BITS
    MPIR_FALLOC(old,(MPI_Datatype *)MALLOC( *count * sizeof(MPI_Datatype) ),
		MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		"Out of space in MPI_TYPE_STRUCT" );
    for (i=0; i<*count; i++) {
	old[i] = (MPI_Datatype)MPIR_ToPointer(old_types[i]);
    }
    *__ierr = MPI_Type_struct(*count,blocklens,c_indices,old,&lnewtype);
    FREE( old );
#else

    *__ierr = MPI_Type_struct(*count,blocklens,c_indices,old_types,
				  &lnewtype);
#endif
    
    FREE( c_indices );
    *(int*)newtype = MPIR_FromPointer(lnewtype);
}
