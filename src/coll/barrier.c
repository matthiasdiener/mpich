/*
 *  $Id: barrier.c,v 1.17 1994/12/15 17:28:35 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: barrier.c,v 1.17 1994/12/15 17:28:35 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Barrier - Blocks until all process have reached this routine.

Input Parameters:
. comm - communicator (handle) 

Notes:
Blocks the caller until all group members have called it; 
the call returns at any process only after all group members
have entered the call.

Algorithm:  
A tree-like or combine algorithm is used to broadcast a message 
to all members of the communicator.  We can modifiy this to
use "blocks" at a later time (see MPI_Bcast).

@*/
int MPI_Barrier ( comm )
MPI_Comm comm;
{
  int        rank, size, N2_prev, surfeit;
  int        d, dst, src;
  int        flag;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;

  /* Check for valid communicator */
  if ( MPIR_TEST_COMM(comm,comm) )
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_BARRIER");

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_BARRIER");

  /* Intialize communicator size */
  (void) MPI_Comm_size ( comm, &size );

#ifdef MPID_Barrier
  if (comm->ADIBarrier) {
      MPID_Barrier( comm->ADIctx, comm );
      return MPI_SUCCESS;
      }
#endif
  /* If there's only one member, this is trivial */
  if ( size > 1 ) {

    /* Initialize collective communicator */
    comm = comm->comm_coll;
    (void) MPI_Comm_rank ( comm, &rank );
    (void) MPIR_Comm_N2_prev ( comm, &N2_prev );
    surfeit = size - N2_prev;

    /* Lock for collective operation */
    MPID_THREAD_LOCK(comm->ADIctx,comm);

    /* Perform a combine-like operation */
    if ( rank < N2_prev ) {
      if( rank < surfeit ) {

        /* get the fanin letter from the upper "half" process: */
        dst = N2_prev + rank;

        MPI_Recv((void *)0,0,MPI_INT,dst,MPIR_BARRIER_TAG, comm, &status);
      }

      /* combine on embedded N2_prev power-of-two processes */
      for (d = 1; d < N2_prev; d <<= 1) {
        dst = (rank ^ d);

        MPI_Sendrecv( (void *)0,0,MPI_INT,dst, MPIR_BARRIER_TAG,
                     (void *)0,0,MPI_INT,dst, MPIR_BARRIER_TAG, 
                     comm, &status);
      }

      /* fanout data to nodes above N2_prev... */
      if ( rank < surfeit ) {
        dst = N2_prev + rank;
        MPI_Send( (void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG, comm);
      }
    } 
    else {
      /* fanin data to power of 2 subset */
      src = rank - N2_prev;
      MPI_Sendrecv( (void *)0, 0, MPI_INT, src, MPIR_BARRIER_TAG,
                   (void *)0, 0, MPI_INT, src, MPIR_BARRIER_TAG, 
                   comm, &status);
    }

    /* Unlock for collective operation */
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  } 
  return(MPI_SUCCESS); 
}




