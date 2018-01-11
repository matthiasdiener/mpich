/*
 *  $Id: gather.c,v 1.25 1996/04/12 15:39:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Gather - Gathers together values from a group of processes
 
Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements for any single receive (integer, 
significant only at root) 
. recvtype - data type of recv buffer elements 
(significant only at root) (handle) 
. root - rank of receiving process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, significant only at 'root') 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Gather ( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, 
		 root, comm )
void             *sendbuf;
int               sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcount;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcnt) ||
       MPIR_TEST_DATATYPE(comm,sendtype) ) 
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_GATHER" );

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Gather(sendbuf, sendcnt, sendtype, 
			       recvbuf, recvcount, recvtype, 
			       root, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_GATHER");
}
