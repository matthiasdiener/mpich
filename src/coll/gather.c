/*
 *  $Id: gather.c,v 1.21 1994/12/15 17:29:10 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: gather.c,v 1.21 1994/12/15 17:29:10 gropp Exp $";
#endif /* lint */

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
. recvbuf - address of receive buffer (choice, significant only at root) 

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
  MPI_Status status;
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  int        flag;
  int        mask, relrank, source, len, offset, totalcnt, count;
  char       *buffer;
  MPI_Aint   extent;

  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcnt) ||
       MPIR_TEST_DATATYPE(comm,sendtype) ) 
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_GATHER" );

  /* Check for Intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag)
    return MPIR_ERROR(comm, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_GATHER");
  
  /* Is root within the communicator? */
  MPI_Comm_size ( comm, &size );
  if ( (root >= size || root < 0) && (mpi_errno = MPI_ERR_ROOT) )
    return MPIR_ERROR( comm, mpi_errno, "Invalid root in MPI_GATHER" );

  /* Get my rank and switch communicators to the hidden collective */
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If rank == root, then I recv lots, otherwise I send */
  /* This should use the same mechanism used in reduce; the intermediate nodes
     will need to allocate space. 

     Here's the algorithm (we'll use it soon).  
     Relative to the root, look at the bit pattern in 
     my rank.  Starting from the right (lsb), if the bit is 1, send to 
     the node with that bit zero and exit; if the bit is 0, receive from the
     node with that bit set and combine (as long as that node is within the
     group)

     Note that by receiveing with source selection, we guarentee that we get
     each contribution to the buffer in turn.  The size that is sent doubles
     at each step (if there are a power of two number of processors).
     Because of the way in which these are ordered, the low processor always
     gets some data to add to the end of its buffer; it never has to worry 
     about interleaving the data or copying the data from a temporary
     buffer into a final buffer.

     To see this, note that the ordering is
     (ab)(cd)(ef)(gh)        -> a c e g
     ((ab)(cd))((ef)(gh))    -> a   e
     (((ab)(cd))((ef)(gh)))  -> a
   */
#ifdef FOO
	 /* Problems with this:
	    recvcount/recvtype known only at root!
	    Internal receives should use a contiguous version of the 
	    datatype.
          */
  MPI_Type_extent ( recvtype, &extent );
  /* we can actually use less space; we'll never need more than half of
     this and if we are low in the chain (for example, our relative rank
     is odd), we may not need any buffer at all. */
  buffer = (char *)MALLOC( extent * recvcount * size );
  if (!buffer) {
      return MPIR_ERROR(comm, MPI_ERR_EXHAUSTED, 
			"Out of space in MPI_GATHER" );
      }
  mask    = 0x1;

  offset = extent*count;
  memcpy( buffer, sendbuf, offset );

  relrank = (rank - root + size) % size;
  totalcnt = count;
  while ((mask & relrank) == 0 && mask < size) {
      /* Receive */
      source = ((relrank | mask) + root) % size;
      if (source < size) {
	  mpi_errno = MPI_Recv (buffer+offset, count*size-totalcnt, 
			    recvtype, source, 
			    MPIR_GATHER_TAG, comm, &status);
	  if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, 
				       "Error receiving in MPI_REDUCE" );
	  MPI_Get_count( &status, recvtype, &len );
	  offset   += len * extent;
	  totalcnt += len;
	  }
      mask <<= 1;
      }
  if (mask < size) {
      source = ((relrank & (~ mask)) + root) % size;
      mpi_errno  = MPI_Send( buffer, totalcnt, sendtype, source, 
			     MPIR_GATHER_TAG, 
			 comm );
      if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, 
				   "Error sending in MPI_REDUCE" );
      }
  FREE( buffer );
  
#else
  if ( rank == root ) {
    int         i;
    MPI_Request req;
    MPI_Status  status;

    MPI_Isend(sendbuf, sendcnt, sendtype, root, MPIR_GATHER_TAG, comm, &req);
    MPI_Type_extent(recvtype, &extent);
    for ( i=0; i<size; i++ ) {
      MPI_Recv( (void *)(((char*)recvbuf)+i*extent*recvcount), 
			   recvcount, recvtype, i, 
			   MPIR_GATHER_TAG, comm, &status);
    }
	MPI_Wait(&req, &status);
  }
  else 
    MPI_Send(sendbuf, sendcnt, sendtype, root, MPIR_GATHER_TAG, comm);
#endif  

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

