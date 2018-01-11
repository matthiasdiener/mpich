/*
 *  $Id: getpnamef.c,v 1.12 1996/01/29 17:52:02 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: getpnamef.c,v 1.12 1996/01/29 17:52:02 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#ifdef _CRAY
#include <fortran.h>
#endif

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
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
MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, *len );
*len  = strlen( name );
*ierr = MPI_SUCCESS;
}

#else
void mpi_get_processor_name_( name, len, ierr, d )
int *len, *ierr, d;
char *name;
{
MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, d );
*len  = strlen( name );
*ierr = MPI_SUCCESS;
}
#endif
