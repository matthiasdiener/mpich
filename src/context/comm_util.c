/*
 *  $Id: comm_util.c,v 1.36 1995/09/13 21:43:49 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpiimpl.h"
#include "mpisys.h"

void MPIR_Comm_collops_init ANSI_ARGS((MPI_Comm, MPIR_COMM_TYPE));
/*

MPIR_Comm_make_coll - make a hidden collective communicator
                      from an inter- or intra-communicator assuming
		      that an appropriate number of contexts
		      have been allocated.  An inter-communicator
		      collective can only be made from another
		      inter-communicator.

See comm_create.c for code that creates a visible communicator.
*/
int MPIR_Comm_make_coll ( comm, comm_type )
MPI_Comm       comm;
MPIR_COMM_TYPE comm_type;
{
  MPI_Comm new_comm         = NEW(struct MPIR_COMMUNICATOR);
  int      mpi_errno;

  if (!new_comm) 
      return MPIR_ERROR(comm, MPI_ERR_EXHAUSTED,
			"Error creating new communicator" );

  (void) MPIR_Comm_init( new_comm, comm, comm_type );
  MPIR_Attr_dup_tree ( comm, new_comm );

  if (comm_type == MPIR_INTRA) {
    new_comm->recv_context    = comm->recv_context + 1;
    new_comm->send_context    = new_comm->recv_context;
    MPIR_Group_dup ( comm->local_group, &(new_comm->group) );
    MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  }
  else {
    new_comm->recv_context    = comm->recv_context + 1;
    new_comm->send_context    = comm->send_context + 1;
    MPIR_Group_dup ( comm->group, &(new_comm->group) );
    MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  }
  new_comm->local_rank     = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;
  new_comm->collops        = NULL;

  new_comm->comm_coll       = new_comm;  /* a circular reference to myself */
  comm->comm_coll           = new_comm;

  /* Place the same operations on both the input (comm) communicator and
     the private copy (new_comm) */
  MPIR_Comm_collops_init( new_comm, comm_type);
  MPIR_Comm_collops_init( comm, comm_type);

  /* The MPID_Comm_init routine needs the size of the local group, and
     reads it from the new_comm structure */
  if (mpi_errno = MPID_Comm_init( new_comm->ADIctx, comm, new_comm )) 
      return mpi_errno;
  
  MPID_THREAD_LOCK_INIT(new_comm->ADIctx,new_comm);
  return(MPI_SUCCESS);
}


/*+

MPIR_Comm_N2_prev - retrieve greatest power of two < size of Comm.

+*/
int MPIR_Comm_N2_prev ( comm, N2_prev )
MPI_Comm comm;
int              *N2_prev;
{
  (*N2_prev) = comm->group->N2_prev;
  return (MPI_SUCCESS);
}


/*+
  MPIR_Dump_comm - utility function to dump a communicator 
+*/
int MPIR_Dump_comm ( comm )
MPI_Comm comm;
{
  int  rank;

  MPIR_Comm_rank ( MPI_COMM_WORLD, &rank );

  printf("[%d] ----- Dumping communicator -----\n", rank );
  if (comm->comm_type == MPIR_INTRA) {
    printf("[%d] Intra-communicator\n",rank);
    printf("[%d] Group\n",rank);
    MPIR_Dump_group ( comm->group );
  }
  else {
    printf("[%d]\tInter-communicator\n",rank);
    printf("[%d] Local group\n",rank);
    MPIR_Dump_group ( comm->local_group );
    printf("[%d] Remote group\n",rank);
    MPIR_Dump_group ( comm->group );
  }
  printf ("[%d] Ref count = %d\n",rank,comm->ref_count);
  printf ("[%d] Send = %d   Recv =%d\n",
          rank,comm->send_context,comm->recv_context);
  printf ("[%d] permanent = %d\n",rank,comm->permanent);
  return (MPI_SUCCESS);
}

/*+
  MPIR_Intercomm_high - determine a high value for an
                        inter-communicator
+*/
int MPIR_Intercomm_high ( comm, high )
MPI_Comm  comm;
int      *high;
{
  MPI_Status status;
  MPI_Comm   inter = comm->comm_coll;
  MPI_Comm   intra = inter->comm_coll;
  int        rank, rhigh;

  MPIR_Comm_rank ( comm, &rank );

  /* Node 0 determines high value */
  if (rank == 0) {

    /* "Normalize" value for high */
    if (*high)
      (*high) = 1;
    else
      (*high) = 0;

    /* Get the remote high value from remote node 0 and determine */
    /* appropriate high */
    MPI_Sendrecv(  high, 1, MPI_INT, 0, 0, 
                 &rhigh, 1, MPI_INT, 0, 0, inter, &status);
    if ( (*high) == rhigh ) {
      if ( comm->group->lrank_to_grank[0] < 
           comm->local_group->lrank_to_grank[0] )
        (*high) = 1;
      else
        (*high) = 0;
    }
  }

  /* Broadcast high value to all */
  MPI_Bcast ( high, 1, MPI_INT, 0, intra );
  return (MPI_SUCCESS);
}


/*

MPIR_Comm_init  - Initialize some of the elements of a communicator from 
                  an existing one.
*/
int MPIR_Comm_init ( new_comm, comm, comm_type )
MPI_Comm       new_comm, comm;
MPIR_COMM_TYPE comm_type;
{
  int      mpi_errno;

  MPIR_SET_COOKIE(new_comm,MPIR_COMM_COOKIE);
  new_comm->ADIctx      = comm->ADIctx;
  new_comm->comm_type   = comm_type;
  new_comm->comm_cache      = 0;
  new_comm->error_handler   = 0;
  MPI_Errhandler_set( new_comm, comm->error_handler );
  new_comm->ref_count       = 1;
  new_comm->permanent       = 0;
  new_comm->collops         = NULL;
  return(MPI_SUCCESS);
}

/* Init the collective ops functions.
 * Default to the ones MPIR provides.
 */

void MPIR_Comm_collops_init( comm, comm_type )
MPI_Comm comm;
MPIR_COMM_TYPE comm_type;
{  
    comm->collops = (comm_type == MPIR_INTRA) ? MPIR_intra_collops :
                                                MPIR_inter_collops ;
    /* Here, we know that these collops are static, but it is still
     * useful to keep the ref count, because it avoids explicit checks
     * when we free them 
     */
    comm->collops->ref_count++;
}
