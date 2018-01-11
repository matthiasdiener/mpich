/*
 *  $Id: alltoall.c,v 1.27 1996/04/12 14:14:59 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Alltoall - Sends data from all to all processes

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements received from any process (integer) 
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

.N fortran

.N Errors
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Alltoall( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcnt;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int          mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcount) ||
       MPIR_TEST_COUNT(comm,recvcnt) || MPIR_TEST_DATATYPE(comm,sendtype) ||
       MPIR_TEST_DATATYPE(comm,recvtype) )
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLTOALL" ); 
  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Alltoall(sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_ALLTOALL");
}
