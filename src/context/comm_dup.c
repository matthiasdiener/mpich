/*
 *  $Id: comm_dup.c,v 1.36 1997/01/07 01:47:16 gropp Exp $
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
  struct MPIR_COMMUNICATOR *new_comm, *comm_ptr;
  int mpi_errno;
  MPIR_ERROR_DECL;
  static char myname[] = "MPI_COMM_DUP";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  /* Check for non-null communicator */
  if ( MPIR_TEST_COMM_NOTOK(comm,comm_ptr) ) {
      (*comm_out) = MPI_COMM_NULL;
      return MPIR_ERROR( comm_ptr, MPI_ERR_COMM, myname);
  }

  /* Duplicate the communicator */
  MPIR_ALLOC(new_comm,NEW(struct MPIR_COMMUNICATOR),comm_ptr,MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_COMM_DUP" );
    MPIR_Comm_init( new_comm, comm_ptr, comm_ptr->comm_type );
  MPIR_Group_dup ( comm_ptr->group,       &(new_comm->group) );
  MPIR_Group_dup ( comm_ptr->local_group, &(new_comm->local_group) );
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
  /* Also free at least some of the parts of the commuicator */      
  if ((mpi_errno = MPIR_Attr_copy ( comm_ptr, new_comm ) )) {
      MPI_Group gtmp1, gtmp2;
      *comm_out = MPI_COMM_NULL;
      /* This should really use a more organized "delete-incomplete-object"
	 call */
      gtmp1 = new_comm->group->self;
      gtmp2 = new_comm->local_group->self;
      MPI_Group_free( &gtmp1 );
      MPI_Group_free( &gtmp2 );
      MPI_Errhandler_free( &new_comm->error_handler );
      MPIR_CLR_COOKIE(new_comm);
      MPIR_RmPointer( new_comm->self );
      FREE( new_comm );
      return MPIR_ERROR( comm_ptr, mpi_errno, "Error copying attributes" );
  }

  /* Duplicate intra-communicators */
  if ( comm_ptr->comm_type == MPIR_INTRA ) {
	(void) MPIR_Context_alloc ( comm_ptr, 2, &(new_comm->send_context) );
	new_comm->recv_context    = new_comm->send_context;
	DBG(FPRINTF(OUTFILE,"Dup:About to make collcomm for %ld\n",(long)new_comm);)
	(void) MPIR_Comm_make_coll ( new_comm, MPIR_INTRA );
  }

  /* Duplicate inter-communicators */
  else {
	struct MPIR_COMMUNICATOR *inter_comm = comm_ptr->comm_coll;
	struct MPIR_COMMUNICATOR *intra_comm = comm_ptr->comm_coll->comm_coll;
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
				    inter_comm->self, &status);
	  MPIR_ERROR_POP(inter_comm);
	  if (mpi_errno) return MPIR_ERROR(comm_ptr,mpi_errno, myname );
	}
	
	/* Broadcast the send context */
	MPIR_ERROR_PUSH(intra_comm);
	mpi_errno = MPI_Bcast(&send_context, 1, MPIR_CONTEXT_TYPE, 0, 
			      intra_comm->self);
	MPIR_ERROR_POP(intra_comm);
	if (mpi_errno) return MPIR_ERROR(comm_ptr,mpi_errno,myname );

	/* We all now have all the information necessary,finish building the */
	/* inter-communicator */
	new_comm->send_context  = send_context;
	new_comm->recv_context  = recv_context;

	/* Build the collective inter-communicator */
	MPIR_Comm_make_coll( new_comm, MPIR_INTER );

	/* Build the collective intra-communicator */
	MPIR_Comm_make_coll ( new_comm->comm_coll, MPIR_INTRA );
  }
  (*comm_out)               = new_comm->self;

  /* Remember it for the debugger */
  MPIR_Comm_remember ( new_comm );

  DBG(FPRINTF(OUTFILE,"Dup:done for new comm %ld\n", (long)new_comm );)
  TR_POP;
  return(MPI_SUCCESS);
}
