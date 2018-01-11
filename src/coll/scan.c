/*
 *  $Id: scan.c,v 1.27 1995/02/06 22:23:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: scan.c,v 1.27 1995/02/06 22:23:46 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

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
@*/
int MPI_Scan ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  MPI_Status status;
  int        rank, size;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Aint   extent;
  MPI_User_function   *uop;
  int        flag; 

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       ( ((count>0)&&(sendbuf==(void *)0))  && (mpi_errno = MPI_ERR_BUFFER) ) ||
       ( ((count>0)&&(recvbuf==(void *)0))  && (mpi_errno = MPI_ERR_BUFFER) ) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf))
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_SCAN" );
  /* We also need to check that the datatype is a basic type? */

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
					  "Inter-communicator invalid in MPI_SCAN");

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* Get my rank & size and switch communicators to the hidden collective */
  MPI_Comm_size ( comm, &size );
  MPI_Comm_rank ( comm, &rank );
  MPI_Type_extent ( datatype, &extent );
  comm = comm->comm_coll;
  uop = op->op;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* 
     This is an O(size) algorithm.  A modification of the algorithm in 
     reduce.c can be used to make this O(log(size)) 
   */
  /* commutative case requires no extra buffering */
  if (op->commute) {
      /* Do the scan operation */
      if (rank > 0) {
          MPI_Recv(recvbuf,count,datatype,rank-1,MPIR_SCAN_TAG,comm,&status);
          (*uop)(sendbuf, recvbuf, &count, &datatype); 
      }
      else {
          MPI_Sendrecv(sendbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       recvbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       comm, &status);
      }
  }
  /* non-commutative case requires extra buffering */
  else {
      /* Do the scan operation */
      if (rank > 0) {
          int size;
          void *tmpbuf;
          tmpbuf = (void *)MALLOC(extent * count);
          if (!tmpbuf) {
              return MPIR_ERROR(comm, MPI_ERR_EXHAUSTED, 
                                "Out of space in MPI_SCAN" );
          }
          MPI_Sendrecv(sendbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       recvbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       comm, &status);
          MPI_Recv(tmpbuf,count,datatype,rank-1,MPIR_SCAN_TAG,comm,&status);
          (*uop)(tmpbuf, recvbuf, &count, &datatype); 
          FREE(tmpbuf);
      }
      else {
          MPI_Sendrecv(sendbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       recvbuf,count,datatype,rank, MPIR_SCAN_TAG,
                       comm, &status);
      }
  }

  /* send the letter to destination */
  if (rank < (size-1)) 
    MPI_Send(recvbuf,count,datatype,rank+1,MPIR_SCAN_TAG,comm);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return(mpi_errno);
}
