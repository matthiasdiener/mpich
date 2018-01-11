/*
 *  $Id: comm_dup.c,v 1.32 1996/06/26 19:27:26 gropp Exp $
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

#define DBG(a)
#define OUTFILE stdout

/*@

MPI_Comm_dup - Duplicates an existing communicator with all its cached
               information

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. newcomm - copy of 'comm' (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Comm_free
@*/
int MPI_Comm_dup ( comm, comm_out )
MPI_Comm comm, *comm_out;
{
  MPI_Comm new_comm;
  int mpi_errno;
  MPIR_ERROR_DECL;

  /* Check for non-null communicator */
  if ( MPIR_TEST_COMM(comm,comm) ) {
    (*comm_out) = MPI_COMM_NULL;
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_COMM_DUP" );
  }

  /* Duplicate the communicator */
  MPIR_ALLOC(new_comm,NEW(struct MPIR_COMMUNICATOR),comm,MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_COMM_DUP" );

  (void) MPIR_Comm_init( new_comm, comm, comm->comm_type );
  (void) MPIR_Group_dup ( comm->group,       &(new_comm->group) );
  (void) MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  new_comm->local_rank     = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;
  new_comm->comm_name	   = 0;
#ifdef MPI_ADI2
    if ((mpi_errno = MPID_CommInit( comm, new_comm )))
	return mpi_errno;
#else
  if ((mpi_errno = MPID_Comm_init( new_comm->ADIctx, comm, new_comm ))) 
      return mpi_errno;
#endif
  DBG(FPRINTF(OUTFILE,"Dup:About to copy attr for comm %ld\n",(long)comm);)
  if ((mpi_errno = MPIR_Attr_copy ( comm, new_comm ) ))
      return MPIR_ERROR( comm, mpi_errno, "Error copying attributes" );

  /* Duplicate intra-communicators */
  if ( comm->comm_type == MPIR_INTRA ) {
	(void) MPIR_Context_alloc ( comm, 2, &(new_comm->send_context) );
	new_comm->recv_context    = new_comm->send_context;
	DBG(FPRINTF(OUTFILE,"Dup:About to make collcomm for %ld\n",(long)new_comm);)
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
	MPIR_ERROR_PUSH(inter_comm);
	MPIR_Comm_rank ( intra_comm, &rank );
	if (rank == 0) {
	  MPI_Status status;
	  
	  MPIR_ERROR_PUSH(inter_comm);
	  mpi_errno = MPI_Sendrecv (&recv_context, 
				    1, MPIR_CONTEXT_TYPE, 0, MPIR_IC_DUP_TAG,
				    &send_context, 
				    1, MPIR_CONTEXT_TYPE, 0, MPIR_IC_DUP_TAG,
				    inter_comm, &status);
	  MPIR_ERROR_POP(inter_comm);
	  if (mpi_errno) return MPIR_ERROR(comm,mpi_errno,
					   "Error in MPI_COMM_DUP");
	}
	
	/* Broadcast the send context */
	MPIR_ERROR_PUSH(intra_comm);
	mpi_errno = MPI_Bcast(&send_context, 1, MPIR_CONTEXT_TYPE, 0, 
			      intra_comm);
	MPIR_ERROR_POP(intra_comm);
	if (mpi_errno) return MPIR_ERROR(comm,mpi_errno,
					   "Error in MPI_COMM_DUP");

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

  /* Remember it for the debugger */
  MPIR_Comm_remember ( new_comm );

  DBG(FPRINTF(OUTFILE,"Dup:done for new comm %ld\n", (long)new_comm );)
  return(MPI_SUCCESS);
}
