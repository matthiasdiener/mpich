/*
 *  $Id: barrier.c,v 1.20 1996/04/12 14:15:10 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

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

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
@*/
int MPI_Barrier ( comm )
MPI_Comm comm;
{
  int        mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  /* Check for valid communicator before use */
  if ( MPIR_TEST_COMM(comm,comm) )
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_BARRIER");

  MPIR_ERROR_PUSH(comm);
  mpi_errno = comm->collops->Barrier(comm);
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_BARRIER");
}




