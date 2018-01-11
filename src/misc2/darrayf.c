/* 
 *   $Id: darrayf.c,v 1.4 1998/04/29 16:59:11 swider Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_create_darray_ PMPI_TYPE_CREATE_DARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_darray_ pmpi_type_create_darray__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_type_create_darray pmpi_type_create_darray_
#endif
#define mpi_type_create_darray_ pmpi_type_create_darray
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_type_create_darray_ pmpi_type_create_darray
#endif
#define mpi_type_create_darray_ pmpi_type_create_darray_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_create_darray_ MPI_TYPE_CREATE_DARRAY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_create_darray_ mpi_type_create_darray__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_type_create_darray mpi_type_create_darray_
#endif
#define mpi_type_create_darray_ mpi_type_create_darray
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_type_create_darray_ mpi_type_create_darray
#endif
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_create_darray_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *,
					MPI_Fint *, MPI_Fint *, MPI_Fint *,
					MPI_Fint *, MPI_Fint *, MPI_Fint *,
					MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_type_create_darray_(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims,
                             MPI_Fint *array_of_gsizes,
			     MPI_Fint *array_of_distribs,
			     MPI_Fint *array_of_dargs,
			     MPI_Fint *array_of_psizes, MPI_Fint *order, 
			     MPI_Fint *oldtype, MPI_Fint *newtype, 
			     MPI_Fint *__ierr )
{
    int i;
    int *l_array_of_gsizes;
    int local_l_array_of_gsizes[MPIR_USE_LOCAL_ARRAY];
    int *l_array_of_distribs;
    int local_l_array_of_distribs[MPIR_USE_LOCAL_ARRAY];
    int *l_array_of_dargs;
    int local_l_array_of_dargs[MPIR_USE_LOCAL_ARRAY];
    int *l_array_of_psizes;
    int local_l_array_of_psizes[MPIR_USE_LOCAL_ARRAY];
    MPI_Datatype oldtype_c, newtype_c;

    oldtype_c = MPI_Type_f2c(*oldtype);

    if ((int)*ndims > 0) {
	if ((int)*ndims > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(l_array_of_gsizes,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_DARRAY" );

	    MPIR_FALLOC(l_array_of_distribs,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_DARRAY" );

	    MPIR_FALLOC(l_array_of_dargs,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_DARRAY" );

	    MPIR_FALLOC(l_array_of_psizes,(int *) MALLOC( *ndims * sizeof(int) ), 
			MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
			"MPI_TYPE_CREATE_DARRAY" );
	}
	else {
	    l_array_of_gsizes = local_l_array_of_gsizes;
	    l_array_of_distribs = local_l_array_of_distribs;
	    l_array_of_dargs = local_l_array_of_dargs;
	    l_array_of_psizes = local_l_array_of_psizes;
	}

	for (i=0; i<(int)*ndims; i++) {
	    l_array_of_gsizes[i] = (int)array_of_gsizes[i];
	    l_array_of_distribs[i] = (int)array_of_distribs[i];
	    l_array_of_dargs[i] = (int)array_of_dargs[i];
	    l_array_of_psizes[i] = (int)array_of_psizes[i];
	}
    }

    *__ierr = MPI_Type_create_darray((int)*size, (int)*rank, (int)*ndims,
				     l_array_of_gsizes, l_array_of_distribs,
				     l_array_of_dargs, l_array_of_psizes,
				     (int)*order, oldtype_c, &newtype_c);

    if ((int)*ndims > MPIR_USE_LOCAL_ARRAY) {
	FREE( l_array_of_gsizes );
	FREE( l_array_of_distribs );
	FREE( l_array_of_dargs );
	FREE( l_array_of_psizes );
    }

    *newtype = MPI_Type_c2f(newtype_c);
}

