/* type_get_cont.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */

#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_get_contents_ PMPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_contents_ pmpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_contents_ pmpi_type_get_contents
#else
#define mpi_type_get_contents_ pmpi_type_get_contents_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_get_contents_ MPI_TYPE_GET_CONTENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_contents_ mpi_type_get_contents__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_contents_ mpi_type_get_contents
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_get_contents_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint *, MPI_Fint *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_type_get_contents_(datatype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes, __ierr )
MPI_Fint  *datatype;
MPI_Fint  *max_integers;
MPI_Fint  *max_addresses;
MPI_Fint  *max_datatypes;
MPI_Fint  *array_of_integers;
MPI_Fint  *array_of_addresses;
MPI_Fint  *array_of_datatypes;
MPI_Fint  *__ierr;
{
    int i;
    int *l_array_of_integers;
    MPI_Aint *l_array_of_addresses;
    MPI_Datatype *l_array_of_datatypes;
    
    MPIR_FALLOC(l_array_of_integers, (int*)MALLOC(sizeof(int)* (int)*
		max_integers), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		"MPI_TYPE_GET_CONTENTS");

    MPIR_FALLOC(l_array_of_addresses, (MPI_Aint *)MALLOC(sizeof(MPI_Aint) *
		(int)*max_addresses), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		"MPI_TYPE_GET_CONTENTS");

    MPIR_FALLOC(l_array_of_datatypes, (MPI_Datatype *)MALLOC(sizeof
               (MPI_Datatype) * (int)*max_datatypes), MPIR_COMM_WORLD, 
		MPI_ERR_EXHAUSTED, "MPI_TYPE_GET_CONTENTS");
    
    *__ierr = MPI_Type_get_contents( MPI_Type_f2c(*datatype), 
				     (int)*max_integers, 
				     (int)*max_addresses,
				     (int)*max_datatypes, 
				     l_array_of_integers,
				     l_array_of_addresses,
				     l_array_of_datatypes );

    for (i=0; i<(int)*max_integers; i++)
	array_of_integers[i] = (MPI_Fint)l_array_of_integers[i];
    for (i=0; i<(int)*max_addresses; i++)
	array_of_addresses[i] = (MPI_Aint)l_array_of_addresses[i];
    for (i=0; i<(int)*max_datatypes; i++)
	array_of_datatypes[i] = MPI_Type_c2f(l_array_of_datatypes[i]);

    FREE( l_array_of_integers );
    FREE( l_array_of_addresses );
    FREE( l_array_of_datatypes );
}

