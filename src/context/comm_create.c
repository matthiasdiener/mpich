/*
 *  $Id: comm_create.c,v 1.14 1994/10/24 22:03:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Comm_create - Creates a new communicator

Input Parameters:
. comm - communicator (handle) 
. group - Group, which is a subset of the group of comm  (handle) 

Output Parameter:
. comm_out - new communicator (handle) 

@*/
int MPI_Comm_create ( comm, group, comm_out )
MPI_Comm  comm;
MPI_Group group;
MPI_Comm *comm_out;
{
  int errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || 
	   MPIR_TEST_GROUP(comm,group)||
	   ((comm->comm_type == MPIR_INTER) && (errno = MPI_ERR_COMM))  ) {
    (*comm_out) = MPI_COMM_NULL;
    return MPIR_ERROR( comm, errno, "Error in MPI_COMM_CREATE" );
  }

  /* Create the communicator */
  if (group->local_rank == MPI_UNDEFINED) {
    MPIR_CONTEXT tmp_context;
    /* I'm not in the new communciator but I'll participate in the */
    /* context creation anyway, then deallocate the context that was */
    /* allocated.  I may not need do this, but until I think about */
    /* the consequences a bit more ... */
    (void) MPIR_Context_alloc  ( comm, 2, &tmp_context ); 
    (void) MPIR_Context_dealloc( comm, 2, tmp_context );
    (*comm_out) = MPI_COMM_NULL;
  }
  else {
    MPI_Comm new_comm = (*comm_out) = NEW(struct MPIR_COMMUNICATOR);
    if (!new_comm) 
	  return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED,
					"Out of space in MPI_COMM_CREATE" );
    (void) MPIR_Comm_init( new_comm, comm, MPIR_INTRA );
    (void) MPIR_Group_dup( group, &(new_comm->group) );
	(void) MPIR_Group_dup( group, &(new_comm->local_group) );
    (void) MPIR_Context_alloc( comm, 2, &(new_comm->send_context) );
    new_comm->recv_context = new_comm->send_context;
    (void) MPIR_Attr_create_tree ( new_comm );
    (void) MPIR_Comm_make_coll( new_comm, MPIR_INTRA );
  }
  return(errno);
}
