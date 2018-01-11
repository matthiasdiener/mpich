/*
 *  $Id: errfree.c,v 1.7 1995/12/21 21:56:51 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: errfree.c,v 1.7 1995/12/21 21:56:51 gropp Exp $";
#endif

#include "mpiimpl.h"
#include "mpisys.h"

/*@
  MPI_Errhandler_free - Frees an MPI-style errorhandler

Input Parameter:
. errhandler - MPI error handler (handle).  Set to 'MPI_ERRHANDLER_NULL' on 
exit.

.N fortran
@*/
int MPI_Errhandler_free( errhandler )
MPI_Errhandler *errhandler;
{
    int mpi_errno = MPI_SUCCESS;
    /* Check that value is correct */
    if (MPIR_TEST_ERRHANDLER(MPI_COMM_WORLD,*errhandler))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_ERRHANDLER_FREE" );

    (*errhandler)->ref_count --;
    if ((*errhandler)->ref_count <= 0) {
	MPIR_SET_COOKIE((*errhandler),0);
	MPIR_SBfree ( MPIR_errhandlers, (*errhandler) );
	}
    *errhandler = MPI_ERRHANDLER_NULL;
    return MPI_SUCCESS;
}
