/*
 *  $Id: scatter.c,v 1.24 1996/04/12 15:40:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Scatter - Sends data from one task to all other tasks in a group

Input Parameters:
. sendbuf - address of send buffer (choice, significant 
only at 'root') 
. sendcount - number of elements sent to each process 
(integer, significant only at 'root') 
. sendtype - data type of send buffer elements (significant only at 'root') 
(handle) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - data type of receive buffer elements (handle) 
. root - rank of sending process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Scatter ( sendbuf, sendcnt, sendtype, 
		  recvbuf, recvcnt, recvtype, 
		  root, comm )
void             *sendbuf;
int               sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcnt;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;  

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,sendtype)) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_SCATTER" );

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Scatter(sendbuf, sendcnt, sendtype, 
				recvbuf, recvcnt, recvtype, 
				root, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_SCATTER");
}
