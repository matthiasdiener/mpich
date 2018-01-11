/*
 *  $Id: ic_merge.c,v 1.16 1996/06/26 19:27:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif
#include "ic.h"

/*@

MPI_Intercomm_merge - Creates an intracommuncator from an intercommunicator

Input Parameters:
. comm - Intercommunicator
. high - Used to order the groups of the two intracommunicators within comm
  when creating the new communicator.  

Output Parameter:
. comm_out - Created intracommunicator

.N fortran

Algorithm:
.vb
 1) Allocate two contexts 
 2) Local and remote group leaders swap high values
 3) Determine the high value.
 4) Merge the two groups and make the intra-communicator
.ve

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Intercomm_create, MPI_Comm_free
@*/
int MPI_Intercomm_merge ( comm, high, comm_out )
MPI_Comm  comm;
int       high;
MPI_Comm *comm_out;
{
  int              rank, mpi_errno = MPI_SUCCESS;
  MPI_Comm         new_comm;
  int              flag;
  MPIR_ERROR_DECL;
  static char myname[] = "Error in MPI_INTERCOMM_MERGE";

  /* Check for valid arguments to function */
  if (comm == MPI_COMM_NULL)
    return MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_COMM,
                      "Null communicator in MPI_INTERCOMM_MERGE");

  MPIR_ERROR_PUSH(comm);

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (!flag) MPIR_RETURN_POP(comm,MPI_ERR_COMM,
		       "Intra-communicator invalid in MPI_INTERCOMM_MERGE");

  /* Get my rank in the local group */
  (void) MPIR_Comm_rank ( comm, &rank );

  /* Make the new communicator */
  MPIR_ALLOC_POP(new_comm,NEW(struct MPIR_COMMUNICATOR),comm, 
		 MPI_ERR_EXHAUSTED,"Out of space in MPI_COMM_CREATE" );
  (void) MPIR_Comm_init( new_comm, comm, MPIR_INTRA );
  (void) MPIR_Attr_create_tree ( new_comm );
  MPIR_Comm_collops_init( new_comm, MPIR_INTRA);

  /* Get the high value for our side */
  MPIR_Intercomm_high ( comm, &high );

  /* Merge the two groups according to high value */
  if (high) {
      MPIR_CALL_POP(MPI_Group_union(comm->group, comm->local_group, 
				    &(new_comm->group) ),comm,myname);
  }
  else {
      MPIR_CALL_POP(MPI_Group_union(comm->local_group, comm->group, 
				    &(new_comm->group) ),comm,myname);
  }
  (void) MPIR_Group_dup ( new_comm->group, &(new_comm->local_group) );

#ifndef MPI_ADI2
  MPIR_CALL_POP(MPID_Comm_init( new_comm->ADIctx, comm, new_comm ),comm,
		myname);
#endif
  MPIR_ERROR_POP(comm);

  new_comm->local_rank     = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;

#ifdef MPI_ADI2
  MPIR_CALL_POP(MPID_CommInit( comm, new_comm ),comm,myname);
#endif
  /* Allocate 2 contexts for intra-communicator */
  MPIR_Context_alloc ( comm, 2, &(new_comm->send_context));
  new_comm->recv_context = new_comm->send_context;
  new_comm->comm_name    = 0;

  /* Make the collective communicator */
  (void) MPIR_Comm_make_coll( new_comm, MPIR_INTRA );
  
  (*comm_out) = new_comm;

  /* Remember it for the debugger */
  MPIR_Comm_remember ( new_comm );

  return (mpi_errno);
}
