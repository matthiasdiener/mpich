/* type_get_env.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_get_envelope_ PMPI_TYPE_GET_ENVELOPE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_envelope_ pmpi_type_get_envelope__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_envelope_ pmpi_type_get_envelope
#else
#define mpi_type_get_envelope_ pmpi_type_get_envelope_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_get_envelope_ MPI_TYPE_GET_ENVELOPE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_get_envelope_ mpi_type_get_envelope__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_get_envelope_ mpi_type_get_envelope
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_get_envelope_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint *, MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_type_get_envelope_(datatype, num_integers, num_addresses, num_datatypes, combiner, __ierr )
MPI_Fint  *datatype;
MPI_Fint  *num_integers;
MPI_Fint  *num_addresses;
MPI_Fint  *num_datatypes;
MPI_Fint  *combiner;
MPI_Fint *__ierr;
{
    int l_num_integers;
    int l_num_addresses;
    int l_num_datatypes;
    int l_combiner;

*__ierr = MPI_Type_get_envelope(MPI_Type_f2c(*datatype), &l_num_integers,
				&l_num_addresses, &l_num_datatypes,
				&l_combiner);

    *num_integers = l_num_integers;
    *num_addresses = l_num_addresses;
    *num_datatypes = l_num_datatypes;
    *combiner = l_combiner;

}

