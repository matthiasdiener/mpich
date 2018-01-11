/*
 *  $Id: alltoall.c,v 1.24 1995/05/16 18:10:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: alltoall.c,v 1.24 1995/05/16 18:10:08 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
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
  int          size, rank, i;
  MPI_Aint     send_extent, recv_extent;
  int          mpi_errno = MPI_SUCCESS;
  MPI_Status   status;
  int          flag;
  MPI_Status  *starray;
  MPI_Request *reqarray;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcount) ||
       MPIR_TEST_COUNT(comm,recvcnt) || MPIR_TEST_DATATYPE(comm,sendtype) ||
       MPIR_TEST_DATATYPE(comm,recvtype) )
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLTOALL" ); 

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
			  "Inter-communicator invalid in MPI_ALLTOALL");
  
  /* Get size and rank and switch to collective communicator */
  MPI_Comm_size ( comm, &size );
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  /* Get extent of send and recv types */
  MPI_Type_extent ( sendtype, &send_extent );
  MPI_Type_extent ( recvtype, &recv_extent );

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx, comm);

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
      /* We'd like to avoid sending and receiving to ourselves; 
	 however, this is complicated by the presence of different
	 sendtype and recvtypes. */
      if ( mpi_errno=MPI_Irecv((void *)((char *)recvbuf + i*recvcnt*recv_extent),
                           recvcnt,
                           recvtype,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm,
                           &reqarray[2*i+1])
          )
          break;
      if (mpi_errno=MPI_Isend((void *)((char *)sendbuf+i*sendcount*send_extent),
                           sendcount,
                           sendtype,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm,
                           &reqarray[2*i])
          )
          break;
  }
  
  if (mpi_errno) return mpi_errno;

  /* ... then wait for *all* of them to finish: */
  mpi_errno = MPI_Waitall(2*size,reqarray,starray);
  
  /* clean up */
  FREE(reqarray);
  FREE(starray);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}
