/*
 *  $Id: bcast.c,v 1.29 1996/06/07 15:08:09 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Bcast - Broadcasts a message from the process with rank "root" to
            all other processes of the group. 

Input/output Parameters:
. buffer - starting address of buffer (choice) 
. count - number of entries in buffer (integer) 
. datatype - data type of buffer (handle) 
. root - rank of broadcast root (integer) 
. comm - communicator (handle) 

Algorithm:  
If the underlying device does not take responsibility, this function
uses a tree-like algorithm to broadcast the message to blocks of
processes.  A linear algorithm is then used to broadcast the message
from the first process in a block to all other processes.
'MPIR_BCAST_BLOCK_SIZE' determines the size of blocks.  If this is set
to 1, then this function is equivalent to using a pure tree algorithm.
If it is set to the size of the group or greater, it is a pure linear
algorithm.  The value should be adjusted to determine the most
efficient value on different machines.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
.N MPI_ERR_ROOT
@*/
int MPI_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
MPI_Datatype      datatype;
int               root;
MPI_Comm          comm;
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_ERROR_DECL;

    /* Check for invalid arguments */
    if ( MPIR_TEST_COMM(comm,comm) ||
	 ( (root            <  0)          &&
	     (MPIR_ERROR_PUSH_ARG(&root),mpi_errno = MPI_ERR_ROOT) )) 
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_BCAST" );

    /* See the overview in Collection Operations for why this is ok */
    if (count == 0) return MPI_SUCCESS;
    
    MPIR_ERROR_PUSH(comm);
    mpi_errno = comm->collops->Bcast(buffer, count, datatype, root, comm);
    MPIR_ERROR_POP(comm);
    MPIR_RETURN(comm,mpi_errno,"Error in MPI_BCAST");
}
