/*
 *  $Id: barrier.c,v 1.19 1995/12/21 22:16:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: barrier.c,v 1.19 1995/12/21 22:16:47 gropp Exp $";
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
If the underlying device cannot do better, a tree-like or combine
algorithm is used to broadcast a message wto all members of the
communicator.  We can modifiy this to use "blocks" at a later time
(see 'MPI_Bcast').

.N fortran
@*/
int MPI_Barrier ( comm )
MPI_Comm comm;
{
  int        mpi_errno = MPI_SUCCESS;

  /* Check for valid communicator before use */
  if ( MPIR_TEST_COMM(comm,comm) )
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_BARRIER");

#ifdef MPID_Barrier
  if (comm->ADIBarrier) {
      MPID_Barrier( comm->ADIctx, comm );
      return MPI_SUCCESS;
      }
#endif
  return comm->collops->Barrier(comm);
}




