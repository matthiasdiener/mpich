/*
 *  $Id: errset.c,v 1.8 1996/04/11 20:28:39 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Errhandler_set - Sets the error handler for a communicator

Input Parameters:
. comm - communicator to set the error handler for (handle) 
. errhandler - new MPI error handler for communicator (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Errhandler_set( comm, errhandler )
MPI_Comm       comm;
MPI_Errhandler errhandler;
{
    int mpi_errno;
    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ERRHANDLER(comm,errhandler)) {
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_ERRHANDLER_SET" );
    }
    else {
	if (comm->error_handler) 
	    MPI_Errhandler_free( &comm->error_handler );
	comm->error_handler = errhandler;
	errhandler->ref_count ++;
    }
    return MPI_SUCCESS;
}
