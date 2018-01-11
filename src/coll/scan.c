/*
 *  $Id: scan.c,v 1.31 1996/04/12 15:40:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"
#include "mpiops.h"

/*@

MPI_Scan - Computes the scan (partial reductions) of data on a collection of
           processes

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. count - number of elements in input buffer (integer) 
. datatype - data type of elements of input buffer (handle) 
. op - operation (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - starting address of receive buffer (choice) 

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
int MPI_Scan ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf))
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_SCAN" );

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Scan(sendbuf, recvbuf, count, datatype, 
				  op, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_SCAN");
}
