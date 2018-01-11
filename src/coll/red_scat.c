/*
 *  $Id: red_scat.c,v 1.23 1996/04/12 15:40:19 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"
#include "mpiops.h"

/*@

MPI_Reduce_scatter - Combines values and scatters the results

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. recvcounts - integer array specifying the 
number of elements in result distributed to each process.
Array must be identical on all calling processes. 
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
.N MPI_ERR_OP
.N MPI_ERR_BUFFER_ALIAS
@*/
int MPI_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  int   mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(recvbuf,sendbuf) || MPIR_TEST_DATATYPE(comm,datatype))
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE_SCATTER" );

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Reduce_scatter(sendbuf, recvbuf, recvcnts, 
					    datatype, op, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_REDUCE_SCATTER");
}
