/*
 *  $Id: ic_create.c,v 1.16 1994/12/15 16:39:51 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Intercomm_create - Creates an intercommuncator from two intracommunicators

Input Paramters:
. local_comm - Local (intra)communicator
. local_leader - Rank in local_comm of leader (often 0)
. peer_comm - Remote (intra)communicator
. remote_leader - Rank in peer_comm of leader (often 0)
. tag - Message tag to use in constructing intercommunicator; if multiple
  MPI_Intercomm_creates are being made, they should use different tags (more
  precisely, ensure that the local and remote leaders are using different
  tags for each MPI_intercomm_create).

Output Parameter:
. comm_out - Created intercommunicator

int MPI_Intercomm_create ( local_comm, local_leader, peer_comm, 
                           remote_leader, tag, comm_out )

Algorithm:
. 1) Allocate a send context, an inter-coll context, and an intra-coll context
. 2) Send "send_context" and lrank_to_grank list from local comm group 
     if I'm the local_leader.
. 3) If I'm the local leader, then wait on the posted sends and receives
     to complete.  Post the receive for the remote group information and
	 wait for it to complete.
. 4) Broadcast information received from the remote leader.  
. 5) Create the inter_communicator from the information we now have.
.    An inter-communicator ends up with three levels of communicators. 
     The inter-communicator returned to the user, a "collective" 
     inter-communicator that can be used for safe communications between
     local & remote groups, and a collective intra-communicator that can 
     be used to allocate new contexts during the merge and dup operations.

	 For the resulting inter-communicator, comm_out

       comm_out                       = inter-communicator
	   comm_out->comm_coll            = "collective" inter-communicator
       comm_out->comm_coll->comm_coll = safe collective intra-communicator

@*/
int MPI_Intercomm_create ( local_comm, local_leader, peer_comm, 
                           remote_leader, tag, comm_out )
MPI_Comm  local_comm;
int       local_leader;
MPI_Comm  peer_comm;
int       remote_leader;
int       tag;
MPI_Comm *comm_out;
{
  int              local_size, local_rank, peer_size, peer_rank;
  int              remote_size;
  int              mpi_errno = MPI_SUCCESS;
  MPIR_CONTEXT     context, send_context;
  MPI_Group        remote_group;
  MPI_Comm         new_comm;
  MPI_Request      req[6];
  MPI_Status       status[6];

  /* Check for valid arguments to function */
  (void) MPI_Comm_size ( local_comm, &local_size );
  (void) MPI_Comm_rank ( local_comm, &local_rank );
  (void) MPI_Comm_size ( peer_comm,  &peer_size  );
  (void) MPI_Comm_rank ( peer_comm,  &peer_rank  );
  if (((local_comm    == MPI_COMM_NULL) && (mpi_errno = MPI_ERR_COMM)) ||
      ((peer_comm     == MPI_COMM_NULL) && (mpi_errno = MPI_ERR_COMM)) ||
      ((local_leader  >= local_size)    && (mpi_errno = MPI_ERR_RANK)) || 
      ((local_leader  <  0)             && (mpi_errno = MPI_ERR_RANK)) ||
      ((remote_leader >= peer_size)     && (mpi_errno = MPI_ERR_RANK)) || 
      ((remote_leader <  0)             && (mpi_errno = MPI_ERR_RANK)) ||
      ((peer_rank     == MPI_UNDEFINED) && (mpi_errno = MPI_ERR_RANK)))
    return MPIR_ERROR( local_comm, mpi_errno, 
		       "Error in MPI_INTERCOMM_CREATE" );

  /* Allocate send context, inter-coll context and intra-coll context */
  MPIR_Context_alloc ( local_comm, 3, &context );
  
  /* If I'm the local leader, then exchange information */
  if (local_rank == local_leader) {

    /* Post the receives for the information from the remote_leader */
    /* We don't post a receive for the remote group yet, because we */
    /* don't know how big it is yet. */
    MPI_Irecv (&remote_size, 1, MPI_INT, remote_leader, tag,
               peer_comm, &(req[2]));
    MPI_Irecv (&send_context, 1, MPIR_CONTEXT_TYPE, remote_leader,
               tag, peer_comm, &(req[3]));
    
    /* Send the lrank_to_grank table of the local_comm and an allocated */
    /* context. Currently I use multiple messages to send this info.    */
    /* Eventually, this will change(?) */
    MPI_Isend (&local_size, 1, MPI_INT, remote_leader, tag, 
               peer_comm, &(req[0]));
    MPI_Isend (&context, 1, MPIR_CONTEXT_TYPE, remote_leader, 
               tag, peer_comm, &(req[1]));
    
    /* Wait on the communication requests to finish */
    MPI_Waitall ( 4, req, status );
    
    /* We now know how big the remote group is, so create it */
    remote_group = MPIR_CreateGroup ( remote_size );
    
    /* Post the receive for the group information */
    MPI_Irecv (remote_group->lrank_to_grank, remote_size, MPI_INT,
               remote_leader, tag, peer_comm, &(req[5]));
    
    /* Send the local group info to the remote group */
    MPI_Isend (local_comm->group->lrank_to_grank, local_size, MPI_INT,
               remote_leader, tag, peer_comm, &(req[4]));
    
    /* wait on the send and the receive for the group information */
    MPI_Waitall ( 2, &(req[4]), &(status[4]) );
   
    /* Now we can broadcast the group information to the other local comm */
    /* members. */
    MPI_Bcast(&remote_size,1,MPI_INT,local_rank,local_comm);
    MPI_Bcast(remote_group->lrank_to_grank, remote_size, MPI_INT,
              local_rank, local_comm);
  }
  /* Else I'm just an ordinary comm member, so receive the bcast'd */
  /* info about the remote group */
  else {
	MPI_Bcast(&remote_size, 1, MPI_INT, local_leader,local_comm);
    
   	/* We now know how big the remote group is, so create it */
	remote_group = MPIR_CreateGroup ( remote_size );
	
	/* Receive the group info */
	MPI_Bcast(remote_group->lrank_to_grank, remote_size, MPI_INT, 
			  local_leader, local_comm);
  }

  /* Broadcast the send context */
  MPI_Bcast(&send_context, 1, MPIR_CONTEXT_TYPE, local_leader, local_comm);

  /* We all now have all the information necessary, start building the */
  /* inter-communicator */
  new_comm = (*comm_out) = NEW(struct MPIR_COMMUNICATOR);
  if (!new_comm) 
	return MPIR_ERROR( local_comm, MPI_ERR_EXHAUSTED,
				  "Out of space in MPI_INTERCOMM_CREATE" );
  (void) MPIR_Comm_init( new_comm, local_comm, MPIR_INTER );
  new_comm->group = remote_group;
  (void) MPIR_Group_dup( local_comm->group, &(new_comm->local_group) );
  new_comm->send_context = send_context;
  new_comm->recv_context = context;
  (void) MPIR_Attr_create_tree ( new_comm );

  /* Build the collective inter-communicator */
  MPIR_Comm_make_coll( new_comm, MPIR_INTER );

  /* Build the collective intra-communicator */
  MPIR_Comm_make_coll ( new_comm->comm_coll, MPIR_INTRA );
  
  return (mpi_errno);
}
