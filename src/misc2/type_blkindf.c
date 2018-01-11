/* type_blkind.c */
/* Custom Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_create_indexed_block_ PMPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block
#else
#define mpi_type_create_indexed_block_ pmpi_type_create_indexed_block_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_create_indexed_block_ MPI_TYPE_CREATE_INDEXED_BLOCK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_indexed_block_ mpi_type_create_indexed_block__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_create_indexed_block_ mpi_type_create_indexed_block
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_create_indexed_block_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, 
					       MPI_Fint **, MPI_Fint *,
					       MPI_Fint *, MPI_Fint *));
/* Definitions of Fortran Wrapper routines */
void mpi_type_create_indexed_block_( count, blocklength, 
				     array_of_displacements, old_type, 
				     newtype, __ierr )
MPI_Fint *count;
MPI_Fint *blocklength;
MPI_Fint *array_of_displacements[];
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{

    int i;
    int *l_array_of_displacements;
    int local_l_array_of_displacements[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype lnewtype;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(l_array_of_displacements,(int *) MALLOC( *count * 
			sizeof(int) ), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_INDEXED_BLOCK");
	}
	else {
	    l_array_of_displacements = local_l_array_of_displacements;
	}

	for (i=0; i<(int)*count; i++)
	    l_array_of_displacements[i] = (int)array_of_displacements[i];
    }

    *__ierr = MPI_Type_create_indexed_block((int)*count, (int)*blocklength,
					l_array_of_displacements,
					MPI_Type_c2f( *old_type ),&lnewtype);

    if ((int)*count > MPIR_USE_LOCAL_ARRAY) 
	FREE( l_array_of_displacements );

    *newtype = MPI_Type_c2f( lnewtype );
}




