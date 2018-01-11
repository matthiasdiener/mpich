/*
 *  $Id: ic_merge.c,v 1.8 1994/12/15 16:42:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"
#include "ic.h"

/*@

MPI_Intercomm_merge - Creates an intracommuncator from an intercommunicator

Input Parameters:
. comm - Intercommunicator
. high - Used to order the groups of the two intracommunicators within comm
  when creating the new communicator.  

Output Parameter:
. comm_out - Created intracommunicator

Algorithm:
. 1) Allocate two contexts 
. 2) Local and remote group leaders swap high values
. 3) Determine the high value.
. 4) Merge the two groups and make the intra-communicator

@*/
int MPI_Intercomm_merge ( comm, high, comm_out )
MPI_Comm  comm;
int       high;
MPI_Comm *comm_out;
{
  int              rank, mpi_errno = MPI_SUCCESS;
  MPIR_CONTEXT     context;
  MPI_Comm         new_comm;
  int              flag;

  /* Check for valid arguments to function */
  if (comm == MPI_COMM_NULL)
    return MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_COMM,
                      "Null communicator in MPI_INTERCOMM_MERGE");

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (!flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
					  "Intra-communicator invalid in MPI_INTERCOMM_MERGE");

  /* Get my rank in the local group */
  (void) MPI_Comm_rank ( comm, &rank );

  /* Make the new communicator */
  new_comm =  NEW(struct MPIR_COMMUNICATOR);
  if (!new_comm) 
    return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED,
                      "Out of space in MPI_COMM_CREATE" );
  (void) MPIR_Comm_init( new_comm, comm, MPIR_INTRA );
  (void) MPIR_Attr_create_tree ( new_comm );
  
  /* Get the high value for our side */
  MPIR_Intercomm_high ( comm, &high );

  /* Merge the two groups according to high value */
  if (high) 
	MPI_Group_union(comm->group, comm->local_group, &(new_comm->group) );
  else 
	MPI_Group_union(comm->local_group, comm->group, &(new_comm->group) );
  MPIR_Group_dup ( new_comm->group, &(new_comm->local_group) );

  /* Allocate 2 contexts for intra-communicator */
  MPIR_Context_alloc ( comm, 2, &(new_comm->send_context));
  new_comm->recv_context = new_comm->send_context;

  /* Make the collective communicator */
  (void) MPIR_Comm_make_coll( new_comm, MPIR_INTRA );
  
  (*comm_out) = new_comm;

  return (mpi_errno);
}
