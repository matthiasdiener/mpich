/*
 *  $Id: reduce.c,v 1.35 1996/04/12 15:40:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"
#include "mpiops.h"

/*@

MPI_Reduce - Reduces values on all processes to a single value

Input Parameters:
. sendbuf - address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - reduce operation (handle) 
. root - rank of root process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, 
significant only at 'root') 

Algorithm:
This implementation currently uses a simple tree algorithm.

.N fortran

.N collops

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
.N MPI_ERR_BUFFER_ALIAS
@*/
int MPI_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
int               root;
MPI_Comm          comm;
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_ERROR_DECL;

    /* Check for invalid arguments */
    if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
	 MPIR_TEST_ALIAS(sendbuf,recvbuf) )
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE" );

    MPIR_ERROR_PUSH(comm);
    mpi_errno = comm->collops->Reduce(sendbuf, recvbuf, count, datatype, 
				      op, root, comm );
    MPIR_ERROR_POP(comm);
    MPIR_RETURN(comm,mpi_errno,"Error in MPI_REDUCE");
}
