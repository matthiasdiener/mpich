/*
 *  $Id: getpnamef.c,v 1.14 1996/06/07 15:12:21 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef _CRAY
#include <fortran.h>
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_get_processor_name_ PMPI_GET_PROCESSOR_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_processor_name_ pmpi_get_processor_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_processor_name_ pmpi_get_processor_name
#else
#define mpi_get_processor_name_ pmpi_get_processor_name_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_processor_name_ MPI_GET_PROCESSOR_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_processor_name_ mpi_get_processor_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_processor_name_ mpi_get_processor_name
#endif
#endif

/*
  MPI_GET_PROCESSOR_NAME - Gets the name of the processor for Fortran

*/
#ifdef _CRAY
void mpi_get_processor_name_( name_fcd, len, ierr )
int *len, *ierr;
_fcd name_fcd;
{
    char *name;
    name = _fcdtocp(name_fcd);
    *len = _fcdlen(name_fcd);
#ifdef MPI_ADI2
    MPID_Node_name( name, MPI_MAX_PROCESSOR_NAME );
#else
    MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, *len );
#endif
    *len  = strlen( name );
    *ierr = MPI_SUCCESS;
}

#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_get_processor_name_ ANSI_ARGS(( char *, int *, int *, int ));

void mpi_get_processor_name_( name, len, ierr, d )
int *len, *ierr, d;
char *name;
{
#ifdef MPI_ADI2
    MPID_Node_name( name, MPI_MAX_PROCESSOR_NAME );
#else
    MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, d );
#endif
    *len  = strlen( name );
    *ierr = MPI_SUCCESS;
}
#endif
