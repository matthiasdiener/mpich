/*
 *  $Id: comm_size.c,v 1.15 1996/04/12 14:06:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*@

MPI_Comm_size - Determines the size of the group associated with a communictor

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. size - number of processes in the group of 'comm'  (integer) 

Notes:
   'MPI_COMM_NULL' is `not` considered a valid argument to this function.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Comm_size ( comm, size )
MPI_Comm comm;
int *size;
{
    int mpi_errno;
    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(size) ) {
	if (size) (*size) = MPI_UNDEFINED;
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_COMM_SIZE" );
    }
    else 
	(*size) = comm->local_group->np;

    return (MPI_SUCCESS);
}
