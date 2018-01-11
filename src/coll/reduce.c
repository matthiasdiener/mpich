/*
 *  $Id: reduce.c,v 1.2 1998/04/28 18:51:01 swider Exp $
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
+ sendbuf - address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - reduce operation (handle) 
. root - rank of root process (integer) 
- comm - communicator (handle) 

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
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    MPIR_ERROR_DECL;
    static char myname[] = "MPI_REDUCE";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    /* Check for invalid arguments */
    if ( MPIR_TEST_ALIAS(sendbuf,recvbuf) )
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

    MPIR_ERROR_PUSH(comm_ptr);
    mpi_errno = comm_ptr->collops->Reduce(sendbuf, recvbuf, count, dtype_ptr, 
					  op, root, comm_ptr );
    MPIR_ERROR_POP(comm_ptr);
    TR_POP;
    MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
