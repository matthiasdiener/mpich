/*
 *  $Id: getpnamef.c,v 1.17 1997/04/08 19:36:49 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
  
/*
 * Update log
 * Nov 29 1996 jcownie@dolphinics.com: Use MPIR_cstr2fstr to get the blank padding right.
 */

#include "mpiimpl.h"
#ifdef _CRAY
#include <fortran.h>
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

#define LOCAL_MIN(a,b) ((a) < (b) ? (a) : (b))

/*
  MPI_GET_PROCESSOR_NAME - Gets the name of the processor for Fortran

*/
#ifdef _CRAY
void mpi_get_processor_name_( name_fcd, len, ierr )
int *len, *ierr;
_fcd name_fcd;
{
    char *name = _fcdtocp(name_fcd);
    long reslen= _fcdlen(name_fcd);
    char cres[MPI_MAX_PROCESSOR_NAME];

#ifdef MPI_ADI2
    MPID_Node_name( cres, MPI_MAX_PROCESSOR_NAME );
#else
    MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, cres, MPI_MAX_PROCESSOR_NAME );
#endif

    /* This handles blank padding required by Fortran */
    MPIR_cstr2fstr(name, reslen, cres );
    *len  = LOCAL_MIN (strlen( cres ), reslen);
    *ierr = MPI_SUCCESS;
}

#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_get_processor_name_ ANSI_ARGS(( char *, int *, int *, int ));

void mpi_get_processor_name_( name, len, ierr, d )
int *len, *ierr, d;
char *name;
{
  char cres[MPI_MAX_PROCESSOR_NAME];

#ifdef MPI_ADI2
    MPID_Node_name( cres, MPI_MAX_PROCESSOR_NAME );
#else
    MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, cres, MPI_MAX_PROCESSOR_NAME );
#endif

    /* This handles blank padding required by Fortran */
    MPIR_cstr2fstr( name, d, cres );
    *len  = LOCAL_MIN( strlen( cres ), d );
    *ierr = MPI_SUCCESS;
}
#endif
