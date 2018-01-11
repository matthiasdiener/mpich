/*
 *  $Id: alltoallv.c,v 1.21 1995/03/27 15:19:36 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: alltoallv.c,v 1.21 1995/03/27 15:19:36 doss Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@

MPI_Alltoallv - Sends data from all to all processes, with a displacement

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcounts - integer array equal to the group size 
specifying the number of elements to send to each processor 
. sdispls - integer array (of length group size). Entry 
 j  specifies the displacement (relative to sendbuf  from
which to take the outgoing data destined for process  j  
. sendtype - data type of send buffer elements (handle) 
. recvcounts - integer array equal to the group size 
specifying the maximum number of elements that can be received from
each processor 
. rdispls - integer array (of length group size). Entry 
 i  specifies the displacement (relative to recvbuf  at
which to place the incoming data from process  i  
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 
@*/
int MPI_Alltoallv ( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm )
void             *sendbuf;
int              *sendcnts;
int              *sdispls;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *rdispls; 
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int        size, rank, i;
  MPI_Aint   send_extent, recv_extent;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  int        flag;
  MPI_Status  *starray;
  MPI_Request *reqarray;
  
  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,sendtype) ||
       MPIR_TEST_DATATYPE(comm,recvtype))
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLTOALLV" ); 

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
			  "Inter-communicator invalid in MPI_ALLTOALLV");
  
  /* Get size and rank and switch to collective communicator */
  MPI_Comm_size ( comm, &size );
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Get extent of send and recv types */
  MPI_Type_extent(sendtype, &send_extent);
  MPI_Type_extent(recvtype, &recv_extent);

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* 1st, get some storage from the heap to hold handles, etc. */
  if (starray = (MPI_Status *)MALLOC(2*size*sizeof(MPI_Status)));
  if (!starray) {
      return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
                         "Error in MPI_ALLTOALL" );
  }
  reqarray = (MPI_Request *)MALLOC(2*size*sizeof(MPI_Request));
  if (!reqarray) {
      FREE(starray );
      return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
                         "Error in MPI_ALLTOALL" );
  }

  /* do the communication -- post *all* sends and receives: */
  for ( i=0; i<size; i++ ) { 
      if ( mpi_errno=MPI_Irecv((void *)((char *)recvbuf+rdispls[i]*recv_extent), 
                           recvcnts[i], 
                           recvtype,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm,
                           &reqarray[2*i+1])
          )
          break;
      if ( mpi_errno=MPI_Isend((void *)((char *)sendbuf+sdispls[i]*send_extent), 
                           sendcnts[i], 
                           sendtype,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm,
                           &reqarray[2*i])
          )
          break;
  }
  
  /* ... then wait for *all* of them to finish: */
  mpi_errno = MPI_Waitall(2*size,reqarray,starray);
  
  /* clean up */
  FREE(reqarray);
  FREE(starray);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}
