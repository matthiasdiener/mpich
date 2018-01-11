/*
 *  $Id: errget.c,v 1.7 1996/04/11 20:28:10 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Errhandler_get - Gets the error handler for a communicator

Input Parameter:
. comm - communicator to get the error handler from (handle) 

Output Parameter:
. errhandler - MPI error handler currently associated with communicator
(handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler *errhandler;
{
    int mpi_errno;
    if (MPIR_TEST_COMM(comm,comm)) {
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_ERRHANDLER_GET" );
    }
    else {
	if (MPIR_TEST_ERRHANDLER(comm,comm->error_handler)) {
	    return MPIR_ERROR( comm, mpi_errno,"Error in MPI_ERRHANDLER_GET" );
	}
	*errhandler = comm->error_handler;
	/* A get creates a reference to an error handler; the user must 
	   explicitly free this reference */
	comm->error_handler->ref_count ++;
    }
    return MPI_SUCCESS;
}
