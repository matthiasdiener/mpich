/*
 *  $Id: bcast.c,v 1.21 1994/09/21 15:27:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: bcast.c,v 1.21 1994/09/21 15:27:41 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Bcast - Broadcasts a message from the process with rank "root" to
            all other processes of the group. 

Input/output Parameter:
. buffer - starting address of buffer (choice) 
. count - number of entries in buffer (integer) 
. datatype - data type of buffer (handle) 
. root - rank of broadcast root (integer) 
. comm - communicator (handle) 

Algorithm:  
This function uses a tree-like algorithm to broadcast 
the message to blocks of processes.  A linear algorithm
is then used to broadcast the message from the first
process in a block to all other processes.  
MPIR_BCAST_BLOCK_SIZE determines the size of blocks.  If
this is set to 1, then this function is equivalent to
using a pure tree algorithm.  If it is set to the
size of the group or greater, it is a pure linear algorithm.
The value should be adjusted to determine the most 
efficient value on different machines.

@*/
int MPI_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
MPI_Datatype      datatype;
int               root;
MPI_Comm          comm;
{
  MPI_Status status;
  int        rank, size, src, dst;
  int        n, N, surfeit, N2_prev, N2_next, N_rank;
  int        participants, my_block, my_offset;
  int        errno = MPI_SUCCESS;
  int        bsize;
  int        int_n;
  int        flag;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) ||
   ( (root            <  0)          && (errno = MPI_ERR_ROOT) )  || 
   ( ((count > 0) && (buffer == (void *)0)) && (errno = MPI_ERR_BUFFER) ) )
    return MPIR_ERROR( comm, errno, "Error in MPI_BCAST" );

  /* Check for Intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag)
    return MPIR_ERROR(comm, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_BCAST");

  /* Is root within the comm and more than 1 processes involved? */
  MPI_Comm_size ( comm, &size );
  if ( (root >= size)  && (errno = MPI_ERR_ROOT) )
    return MPIR_ERROR( comm, errno, "Invalid root in MPI_BCAST" );
  
  /* If there is only one process */
  if (size == 1)
	return (errno);

  /* Get my rank and switch communicators to the hidden collective */
  MPI_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  /* Determine number of blocks and previous power of 2 for number of blocks */
  bsize = (size+MPIR_BCAST_BLOCK_SIZE-1)/MPIR_BCAST_BLOCK_SIZE;
  MPIR_Powers_of_2 ( bsize, &N2_next, &N2_prev );
  participants = MPIR_MIN(size, N2_prev * MPIR_BCAST_BLOCK_SIZE);
  N = N2_prev;

  /* Shift ranks, so root is at virtual node 0 */
  if ( rank < root )
    n = size - root + rank;
  else
    n = rank - root;

  /* How many "extra" nodes do we have? */
  surfeit = size - participants;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If I'm in a participating block */
  if ( n < participants ) {

    /* Which block am I in? and where in the block am I */
    my_block  = n / MPIR_BCAST_BLOCK_SIZE;
    my_offset = n % MPIR_BCAST_BLOCK_SIZE;

    /* If I'm the first node in a participating block, perform a bcast */
    /* using an inter-block binary algorithm and then use a linear */
    /* algorithm to bcast within the block */
    if ( my_offset == 0 ) {
      int num_in_block, i;
	  MPI_Request      hd[MPIR_BCAST_BLOCK_SIZE];
	  MPI_Status       statuses[MPIR_BCAST_BLOCK_SIZE];

      /* Set number of nodes in my block */
      if ((surfeit == 0) && ((my_block+1) == N2_prev) )
		num_in_block = participants - n;
      else
		num_in_block = MPIR_BCAST_BLOCK_SIZE;

	  int_n = n;

      /* While there's someone to send to or someone to receive from ... */
      while ( N > 1 ) {
		
		/* Determine the real rank of first node of middle block */
		N    >>= 1;
		N_rank = N * MPIR_BCAST_BLOCK_SIZE;
		
		/* If I'm the "root" of some processes, then send */
		if ( int_n == 0 ) {
		  dst = (rank + N_rank) % size;
		  MPI_Send (buffer,count,datatype,dst,MPIR_BCAST_TAG,comm);
		}
		/* If a root is sending me a message, then recv and become a 
		   "root" */
		else if ( int_n == N_rank ) {
		  src = (rank - N_rank + size) % size;
		  MPI_Recv(buffer,count,datatype,src,MPIR_BCAST_TAG,comm,
			   &status);
		  int_n   = 0;
		}
		/* I now have a new root to receive from */
		else if ( int_n > N_rank ) {
		  int_n -= N_rank;
		}
		/* else if ( n < N_rank ), do nothing, my root 
		   stayed the same */
	  }
	  
      /* Now linearly broadcast to the other members of my block */
      for ( i = 1; i < num_in_block; i++ ) {
		dst = (rank + i) % size;
		MPI_Isend(buffer,count,datatype,dst,MPIR_BCAST_TAG,comm, 
			  &hd[i]);
      }
	  MPI_Waitall(num_in_block-1, &hd[1], &statuses[1]);
    }
    /* else I'm in a participating block, wait for a message from the first */
    /* node in my block */
    else {
      src = (rank + size - my_offset) % size;
      MPI_Recv(buffer,count,datatype,src,MPIR_BCAST_TAG,comm,&status);
    }
	
    /* Now send to any "extra" nodes */
    if ( n < surfeit ) {
      dst = ( rank + participants ) % size;
      MPI_Send( buffer, count, datatype, dst, MPIR_BCAST_TAG, comm );
    }
  }
  /* else I'm not in a participating block, I'm an "extra" node */
  else {
    src = ( root + n - participants ) % size;
    MPI_Recv ( buffer, count, datatype, src, MPIR_BCAST_TAG, comm, &status );
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (errno);
}
