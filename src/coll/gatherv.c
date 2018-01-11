/*
 *  $Id: gatherv.c,v 1.19 1994/09/29 21:51:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: gatherv.c,v 1.19 1994/09/29 21:51:25 gropp Exp $";
#endif /* lint */

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
(significant only at root) 
. displs - integer array (of length group size). Entry 
 i  specifies the displacement relative to recvbuf  at
which to place the incoming data from process  i  (significant only
at root) 
. recvtype - data type of recv buffer elements 
(significant only at root) (handle) 
. root - rank of receiving process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, significant only at root) 

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
  MPI_Status status;
  int        size, rank;
  int        errno = MPI_SUCCESS;
  int        flag;

  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcnt) ||
       MPIR_TEST_DATATYPE(comm,sendtype)) 
    return MPIR_ERROR(comm, errno, "Error in MPI_GATHERV" );

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_GATHERV");
  
  /* Is root within the communicator? */
  MPI_Comm_size ( comm, &size );
  if ( (root >= size) || (root < 0) )
    return MPIR_ERROR( comm, MPI_ERR_ROOT, "Invalid root in MPI_GATHERV" );

  /* Get my rank and switch communicators to the hidden collective */
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If rank == root, then I recv lots, otherwise I send */
  if ( rank == root ) {
      MPI_Aint       extent;
      int            i;
	MPI_Request req;
	MPI_Status       status;

    MPI_Isend(sendbuf, sendcnt, sendtype, root, MPIR_GATHERV_TAG, comm, &req);
    MPI_Type_extent(recvtype, &extent);
    for ( i=0; i<size; i++ ) {
      MPI_Recv( (void *)((char *)recvbuf+displs[i]*extent), 
	         recvcnts[i], recvtype, i,
		 MPIR_GATHERV_TAG, comm, &status );
    }
	MPI_Wait(&req, &status);
  }
  else 
    MPI_Send( sendbuf, sendcnt, sendtype, root, MPIR_GATHERV_TAG, comm );

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (errno);
}

