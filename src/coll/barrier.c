/*
 *  $Id: barrier.c,v 1.22 1997/01/07 01:47:46 gropp Exp $
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
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_BARRIER";
  MPIR_ERROR_DECL;

  TR_PUSH(myname);

  /* Check for valid communicator before use */
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Barrier(comm_ptr);
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}




