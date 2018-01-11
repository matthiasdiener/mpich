/*
 *  $Id: scatterv.c,v 1.22 1995/05/16 18:09:21 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: scatterv.c,v 1.22 1995/05/16 18:09:21 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Scatterv - Scatters a buffer in parts to all tasks in a group

Input Parameters:
. sendbuf - address of send buffer (choice, significant only at root) 
. sendcounts - integer array (of length group size) 
specifying the number of elements to send to each processor  
. displs - integer array (of length group size). Entry 
 i  specifies the displacement (relative to sendbuf  from
which to take the outgoing data to process  i  
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - data type of receive buffer elements (handle) 
. root - rank of sending process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

@*/
int MPI_Scatterv ( sendbuf, sendcnts, displs, sendtype, 
                   recvbuf, recvcnt,  recvtype, 
                   root, comm )
void             *sendbuf;
int              *sendcnts;
int              *displs;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcnt;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  MPI_Status status;
  int        rank, size;
  int        mpi_errno = MPI_SUCCESS;
  int        flag;

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,sendtype)) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_SCATTER" );

  /* Get size and rank */
  MPI_Comm_size ( comm, &size );
  MPI_Comm_rank ( comm, &rank );

  /* Check for invalid arguments */
  if ( ( (root            <  0)           && (mpi_errno = MPI_ERR_ROOT) )  || 
       ( (root            >= size)        && (mpi_errno = MPI_ERR_ROOT) ))
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_SCATTERV" );

/*
       ( ((recvcnt>0)&&(recvbuf==(void *)0)) && 
	(mpi_errno = MPI_ERR_BUFFER) ) )
 */
  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
					  "Inter-communicator invalid in MPI_SCATTERV");
				  
  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If I'm the root, then scatter */
  if ( rank == root ) {
    MPI_Aint extent;
    int      i;

    MPI_Type_extent(sendtype, &extent);
    for ( i=0; i<root; i++ ) {
      mpi_errno = MPI_Send( (void *)((char *)sendbuf+displs[i]*extent), 
			   sendcnts[i], sendtype, i, MPIR_SCATTERV_TAG, comm);
      if (mpi_errno) return mpi_errno;
      }
    mpi_errno = MPI_Sendrecv((void *)((char *)sendbuf+displs[rank]*extent), 
		 sendcnts[rank], 
                 sendtype, rank, MPIR_SCATTERV_TAG, recvbuf, recvcnt, recvtype, 
                 rank, MPIR_SCATTERV_TAG, comm, &status);
    if (mpi_errno) return mpi_errno;

    for ( i=root+1; i<size; i++ ) {
      mpi_errno = MPI_Send( (void *)((char *)sendbuf+displs[i]*extent), 
			   sendcnts[i], sendtype, i, MPIR_SCATTERV_TAG, comm);
      if (mpi_errno) return mpi_errno;
      }
  }
  else
      mpi_errno = MPI_Recv(recvbuf,recvcnt,recvtype,root,
			   MPIR_SCATTERV_TAG,comm,&status);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


