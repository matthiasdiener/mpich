/*
 *  $Id: allreduce.c,v 1.22 1997/01/07 01:47:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"
#include "mpiops.h"

/*@

MPI_Allreduce - Combines values from all processes and distribute the result
                back to all processes

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - operation (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - starting address of receive buffer (choice) 

.N fortran

.N collops

.N Errors
.N MPI_ERR_BUFFER
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_OP
.N MPI_ERR_COMM
@*/
int MPI_Allreduce ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    MPIR_ERROR_DECL;
    static char myname[] = "MPI_ALLREDUCE";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    /* Check for invalid arguments */
    if (MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_ALIAS(sendbuf,recvbuf))
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );
    
    MPIR_ERROR_PUSH(comm_ptr);
    /* Test for intercommunicator is done when collops is assigned */  
    mpi_errno = comm_ptr->collops->Allreduce(sendbuf, recvbuf, count, 
					     dtype_ptr, op, comm_ptr );
    MPIR_ERROR_POP(comm_ptr);
    TR_POP;
    MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
