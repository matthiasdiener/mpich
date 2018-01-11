/*
 *  $Id: comm_rank.c,v 1.15 1996/04/12 14:04:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@

MPI_Comm_rank - Determines the rank of the calling process in the communicator

Input Parameters:
. comm - communicator (handle) 

Output Parameter:
. rank - rank of the calling process in group of  'comm'  (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
@*/
int MPI_Comm_rank ( comm, rank )
MPI_Comm  comm;
int      *rank;
{
    int mpi_errno;
    if ( MPIR_TEST_COMM(comm,comm) ) {
	(*rank) = MPI_UNDEFINED;
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_COMM_RANK" );
    }
    else 
	(*rank) = comm->local_group->local_rank;

    return (MPI_SUCCESS);
}
