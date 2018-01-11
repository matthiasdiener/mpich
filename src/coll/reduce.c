/*
 *  $Id: reduce.c,v 1.27 1994/12/09 17:40:15 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: reduce.c,v 1.27 1994/12/09 17:40:15 doss Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@

MPI_Reduce - Reduces values on all processes to a single value

Input Parameters:
. sendbuf - address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - reduce operation (handle) 
. root - rank of root process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, 
significant only at root) 

Algorithm:
This implementation currently uses a simple tree algorithm.

@*/
int MPI_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
int               root;
MPI_Comm          comm;
{
  MPI_Status status;
  int        size, rank;
  int        mask, relrank, source, lroot;
  int        errno = MPI_SUCCESS;
  MPI_User_function *uop;
  int        flag;
  MPI_Aint   extent;
  void       *buffer;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf) )
    return MPIR_ERROR(comm, errno, "Error in MPI_REDUCE" );

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
				  "Inter-communicator invalid in MPI_REDUCE");
  
  /* Is root within the communicator? */
  MPI_Comm_size ( comm, &size );
  if ( ((root >= size) || (root < 0)) )
    return MPIR_ERROR(comm, MPI_ERR_ROOT, 
					  "Invalid root in MPI_REDUCE" );

#ifdef MPID_Reduce
  /* Eventually, this could apply the MPID_Reduce routine in a loop for
     counts > 1 */
  if (comm->ADIreduce && count == 1) {
      /* Call a routine to sort through the datatypes and operations ...
	 This allows us to provide partial support (e.g., only SUM_DOUBLE)
       */
      if (MPIR_ADIReduce( comm->ADIctx, comm, sendbuf, recvbuf, count, 
                      datatype, op, root ) == MPI_SUCCESS)
	  return MPI_SUCCESS;
      }
#endif

  /* Get my rank and switch communicators to the hidden collective */
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  uop  = op->op;

  /* Here's the algorithm.  Relative to the root, look at the bit pattern in 
     my rank.  Starting from the right (lsb), if the bit is 1, send to 
     the node with that bit zero and exit; if the bit is 0, receive from the
     node with that bit set and combine (as long as that node is within the
     group)

     Note that by receiveing with source selection, we guarentee that we get
     the same bits with the same input.  If we allowed the parent to receive 
     the children in any order, then timing differences could cause different
     results (roundoff error, over/underflows in some cases, etc).

     Because of the way these are ordered, if root is 0, then this is correct
     for both commutative and non-commutitive operations.  If root is not
     0, then for non-communitive, we use a root of zero and then send
     the result to the root.  To see this, note that the ordering is
     (ab)(cd)(ef)(gh)
     ((ab)(cd))((ef)(gh))
     (((ab)(cd))((ef)(gh)))
   */
  /* Make a temporary buffer */
  MPI_Type_extent ( datatype, &extent );
  buffer = (void *)MALLOC(extent * count);
  if (!buffer) {
	return MPIR_ERROR(comm, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_REDUCE" );
  }

  /* If I'm not the root, then my recvbuf may not be valid, therefore
     I have to allocate a temporary one */
  if (rank != root) {
    recvbuf = (void *)MALLOC(extent * count);
    if (!recvbuf) {
      return MPIR_ERROR(comm, MPI_ERR_EXHAUSTED, 
                        "Out of space in MPI_REDUCE" );
    }
  }

  /* I rename the processes in the following way so that data flows
     from lower numbered to higher numbered processes.  This is needed 
     when the operation is not commutative.
  */
  rank = size - rank - 1;
  root = size - root - 1;

  memcpy( recvbuf, sendbuf, extent*count );
  mask    = 0x1;
  if (op->commute) lroot   = root;
  else             lroot   = 0;
  relrank = (rank - lroot + size) % size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  while ((mask & relrank) == 0 && mask < size) {
	/* Receive */
	source = (relrank | mask);
	if (source < size) {
	  source = (source + lroot) % size;
	  errno = MPI_Recv (buffer, count, datatype, size-source-1, MPIR_REDUCE_TAG, 
						comm, &status);
	  if (errno) return MPIR_ERROR( comm, errno, 
					   "Error receiving in MPI_REDUCE" );
	  (*uop)(buffer, recvbuf, &count, &datatype);
	}
	mask <<= 1;
  }
  if (mask < size) {
	source = ((relrank & (~ mask)) + lroot) % size;
	errno  = MPI_Send( recvbuf, count, datatype, size-source-1, MPIR_REDUCE_TAG, 
					  comm );
	if (errno) return MPIR_ERROR( comm, errno, 
					 "Error sending in MPI_REDUCE" );
  }
  FREE( buffer );
  if (!op->commute && root != 0) {
	if (rank == 0) {
	  errno  = MPI_Send( recvbuf, count, datatype, size-root-1, 
						MPIR_REDUCE_TAG, comm );
	}
	else if (rank == root) {
	  errno = MPI_Recv ( recvbuf, count, datatype, size-1, MPIR_REDUCE_TAG, 
						comm, &status);
	}
  }

  /* Free the temporarily allocated recvbuf */
  if (rank != root)
    FREE( recvbuf );

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (errno);
}









