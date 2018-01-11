/*
 *  $Id: gatherv.c,v 1.24 1996/04/12 15:39:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Gatherv - Gathers into specified locations from all processes in a group

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcounts - integer array (of length group size) 
containing the number of elements that are received from each process
(significant only at 'root') 
. displs - integer array (of length group size). Entry 
 'i'  specifies the displacement relative to recvbuf  at
which to place the incoming data from process  'i'  (significant only
at root) 
. recvtype - data type of recv buffer elements 
(significant only at 'root') (handle) 
. root - rank of receiving process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, significant only at 'root') 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Gatherv ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm )
void             *sendbuf;
int               sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcnt) ||
       MPIR_TEST_DATATYPE(comm,sendtype)) 
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_GATHERV" );

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Gatherv( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_GATHERV");
}
