/*
 *  $Id: comm_dup.c,v 1.25 1994/10/24 22:03:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"
#include "ic.h"

/*@

MPI_Comm_dup - Duplicates an existing communicator with all its cached
               information

Input Parameter:
. comm - communicator (handle) 
Output Parameter:
. newcomm - copy of comm (handle) 

@*/
int MPI_Comm_dup ( comm, comm_out )
MPI_Comm comm, *comm_out;
{
  MPI_Comm new_comm;
  int errno;

  /* Check for non-null communicator */
  if ( MPIR_TEST_COMM(comm,comm) ) {
    (*comm_out) = MPI_COMM_NULL;
	return MPIR_ERROR( comm, errno, "Error in MPI_COMM_DUP" );
  }

  /* Duplicate the communicator */
  new_comm                  = NEW(struct MPIR_COMMUNICATOR);
  if (!new_comm) {
	return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_COMM_DUP" );
  }
  (void) MPIR_Comm_init( new_comm, comm, comm->comm_type );
  (void) MPIR_Group_dup ( comm->group,       &(new_comm->group) );
  (void) MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  (void) MPIR_Attr_copy ( comm, new_comm ); 

  /* Duplicate intra-communicators */
  if ( comm->comm_type == MPIR_INTRA ) {
	(void) MPIR_Context_alloc ( comm, 2, &(new_comm->send_context) );
	new_comm->recv_context    = new_comm->send_context;
	(void) MPIR_Comm_make_coll ( new_comm, MPIR_INTRA );
  }

  /* Duplicate inter-communicators */
  else {
	MPI_Comm     inter_comm = comm->comm_coll;
	MPI_Comm     intra_comm = comm->comm_coll->comm_coll;
	int          rank;
	MPIR_CONTEXT recv_context, send_context;

	/* Allocate send context, inter-coll context and intra-coll context */
	MPIR_Context_alloc ( intra_comm, 3, &recv_context );

	/* If I'm the local leader, then swap context info */
	MPI_Comm_rank ( intra_comm, &rank );
	if (rank == 0) {
	  MPI_Status status;
	  
	  MPI_Sendrecv (&recv_context, 1, MPIR_CONTEXT_TYPE, 0, MPIR_IC_DUP_TAG,
			&send_context, 1, MPIR_CONTEXT_TYPE, 0, MPIR_IC_DUP_TAG,
			inter_comm, &status);
	}
	
	/* Broadcast the send context */
	MPI_Bcast(&send_context, 1, MPIR_CONTEXT_TYPE, 0, intra_comm);

	/* We all now have all the information necessary,finish building the */
	/* inter-communicator */
	new_comm->send_context  = send_context;
	new_comm->recv_context  = recv_context;

	/* Build the collective inter-communicator */
	MPIR_Comm_make_coll( new_comm, MPIR_INTER );

	/* Build the collective intra-communicator */
	MPIR_Comm_make_coll ( new_comm->comm_coll, MPIR_INTRA );
  }
  (*comm_out)               = new_comm;

  return(MPI_SUCCESS);
}
