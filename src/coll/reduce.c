/*
 *  $Id: reduce.c,v 1.32 1995/05/16 18:09:36 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: reduce.c,v 1.32 1995/05/16 18:09:36 gropp Exp $";
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
  int        mpi_errno = MPI_SUCCESS;
  MPI_User_function *uop;
  int        flag;
  MPI_Aint   extent;
  void       *buffer;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf) )
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE" );

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

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

#ifdef MPID_Reduce
  /* Eventually, this could apply the MPID_Reduce routine in a loop for
     counts > 1 */
  if (comm->ADIReduce && count == 1) {
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

     Note that by receiving with source selection, we guarentee that we get
     the same bits with the same input.  If we allowed the parent to receive 
     the children in any order, then timing differences could cause different
     results (roundoff error, over/underflows in some cases, etc).

     Because of the way these are ordered, if root is 0, then this is correct
     for both commutative and non-commutitive operations.  If root is not
     0, then for non-commutitive, we use a root of zero and then send
     the result to the root.  To see this, note that the ordering is
     mask = 1: (ab)(cd)(ef)(gh)            (odds send to evens)
     mask = 2: ((ab)(cd))((ef)(gh))        (3,6 send to 0,4)
     mask = 4: (((ab)(cd))((ef)(gh)))      (4 sends to 0)

     Comments on buffering.  
     If the datatype is not contiguous, we still need to pass contiguous 
     data to the user routine.  
     In this case, we should make a copy of the data in some format, 
     and send/operate on that.

     In general, we can't use MPI_PACK, because the alignment of that
     is rather vague, and the data may not be re-usable.  What we actually
     need is a "squeeze" operation that removes the skips.
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

  /* This code isn't correct if the source is a more complex datatype */
  memcpy( recvbuf, sendbuf, extent*count );
  mask    = 0x1;
  if (op->commute) lroot   = root;
  else             lroot   = 0;
  relrank = (rank - lroot + size) % size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  while (/*(mask & relrank) == 0 && */mask < size) {
	/* Receive */
	if ((mask & relrank) == 0) {
	    source = (relrank | mask);
	    if (source < size) {
		source = (source + lroot) % size;
		mpi_errno = MPI_Recv (buffer, count, datatype, source, 
				      MPIR_REDUCE_TAG, comm, &status);
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, 
					    "Error receiving in MPI_REDUCE" );
		/* The sender is above us, so the received buffer must be
		   the second argument (in the noncommutitive case). */
		if (op->commute)
		    (*uop)(buffer, recvbuf, &count, &datatype);
		else {
		    (*uop)(recvbuf, buffer, &count, &datatype);
		    /* short term hack to keep recvbuf up-to-date */
		    memcpy( recvbuf, buffer, extent*count );
		    }
		}
	    }
	else {
	    /* I've received all that I'm going to.  Send my result to 
	       my parent */
	    source = ((relrank & (~ mask)) + lroot) % size;
	    mpi_errno  = MPI_Send( recvbuf, count, datatype, 
				  source, 
				  MPIR_REDUCE_TAG, 
				  comm );
	    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, 
					     "Error sending in MPI_REDUCE" );
	    break;
	    }
	mask <<= 1;
	}
  FREE( buffer );
  if (!op->commute && root != 0) {
      if (rank == 0) {
	  mpi_errno  = MPI_Send( recvbuf, count, datatype, root, 
				MPIR_REDUCE_TAG, comm );
	  }
      else if (rank == root) {
	  mpi_errno = MPI_Recv ( recvbuf, count, datatype, 0, /*size-1, */
				MPIR_REDUCE_TAG, comm, &status);
	  }
      }

  /* Free the temporarily allocated recvbuf */
  if (rank != root)
    FREE( recvbuf );

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (mpi_errno);
}









