/*
 *  $Id: getversionf.c,v 1.4 1998/01/16 16:25:31 swider Exp $
 *
 *  (C) 1997 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_get_version_ PMPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_version_ pmpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_version_ pmpi_get_version
#else
#define mpi_get_version_ pmpi_get_version_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_version_ MPI_GET_VERSION
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_version_ mpi_get_version__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_version_ mpi_get_version
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_get_version_ ANSI_ARGS(( int *, int *, int * ));

void mpi_get_version_( version, subversion, ierr )
int *version;
int *subversion;
int *ierr;
{
    *version    = MPI_VERSION;
    *subversion = MPI_SUBVERSION;
    *ierr       = MPI_SUCCESS;
}
