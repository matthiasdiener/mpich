/*
 *  $Id: comm_free.c,v 1.37 1995/02/26 14:24:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

#define DBG(a) 

/*@

MPI_Comm_free - Marks the communicator object for deallocation

Input Parameter:
. comm - communicator to be destroyed (handle) 

@*/
int MPI_Comm_free ( comm )
MPI_Comm *comm;
{
  int mpi_errno = MPI_SUCCESS;

  DBG(fprintf(stderr,"About to check for null comm\n");fflush(stderr);)
  /* Check for null communicator */
  if ((*comm) == MPI_COMM_NULL)
    return (mpi_errno);

  DBG(fprintf(stderr,"About to check args\n");fflush(stderr);)
  /* Check for bad arguments */
  if ( MPIR_TEST_COMM(*comm,*comm) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_COMM_FREE" );

  DBG(fprintf(stderr,"About to check for perm comm\n");fflush(stderr);)
  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*comm)->permanent == 1 ) && (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( comm, MPI_ERR_PERM_KEY,
					  "Error in MPI_COMM_FREE" );

  DBG(fprintf(stderr,"About to free group\n");fflush(stderr);)
  /* Free group */
  if ( (*comm)->ref_count <= 1 ) {

        (void)MPID_Comm_free( (*comm)->ADIctx, comm );

        DBG(fprintf(stderr,"About to free context\n");fflush(stderr);)
	/* Free the context used by this communicator */
	(void) MPIR_Context_dealloc ( (*comm), 1, (*comm)->recv_context );
	
        DBG(fprintf(stderr,"About to free cache info\n");fflush(stderr);)
	/* Free cache information */
	MPIR_Attr_free_tree ( *comm );
	
        DBG(fprintf(stderr,"About to finish lock on comm\n");fflush(stderr);)
	/* Free lock on collective comm, if it's not a self-reference */
	if ( (*comm)->comm_coll != (*comm) )
	  MPID_THREAD_LOCK_FINISH((*comm)->ADIctx,(*comm)->comm_coll);

        DBG(fprintf(stderr,"About to free coll comm\n");fflush(stderr);)
	/* Free collective communicator (unless it refers back to myself) */
	if ( (*comm)->comm_coll != (*comm) )
	  MPI_Comm_free ( &((*comm)->comm_coll) );

        DBG(fprintf(stderr,"About to free groups\n");fflush(stderr);)
	/* Free groups */
	MPI_Group_free ( &((*comm)->group) );
	MPI_Group_free ( &((*comm)->local_group) );

	MPI_Errhandler_free( &((*comm)->error_handler) );

        DBG(fprintf(stderr,"About to free comm structure\n");fflush(stderr);)
	/* Free comm structure */
	MPIR_SET_COOKIE((*comm),0);
	FREE( *comm );
  }
  else 
	(*comm)->ref_count--;

  DBG(fprintf(stderr,"About to set comm to comm_null\n");fflush(stderr);)
  /* Set comm to null */
  (*comm) = MPI_COMM_NULL;
	
  return (mpi_errno);
}
