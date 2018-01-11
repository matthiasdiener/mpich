/*
 *  $Id: comm_free.c,v 1.43 1996/06/26 19:27:26 gropp Exp $
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

/* For MPIR_COLLOPS */
#include "mpicoll.h"

#define DBG(a) 
#define OUTFILE stdout

/*@

MPI_Comm_free - Marks the communicator object for deallocation

Input Parameter:
. comm - communicator to be destroyed (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Comm_free ( commp )
MPI_Comm *commp;
{
  int mpi_errno = MPI_SUCCESS;
  MPI_Comm comm = *commp;

  DBG(FPRINTF(OUTFILE, "Freeing communicator %ld\n", (long)comm );)
  DBG(FPRINTF(OUTFILE,"About to check for null comm\n");fflush(OUTFILE);)
  /* Check for null communicator */
  if (comm == MPI_COMM_NULL)
    return (mpi_errno);

  DBG(FPRINTF(OUTFILE,"About to check args\n");fflush(OUTFILE);)
  /* Check for bad arguments */
  if ( MPIR_TEST_COMM(comm,comm) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_COMM_FREE" );

  DBG(FPRINTF(OUTFILE,"About to free group\n");fflush(OUTFILE);)

  if ( --(comm->ref_count) <= 0 ) {

      DBG(FPRINTF(OUTFILE,"About to check for perm comm\n");fflush(OUTFILE);)
      /* We can't free permanent objects unless finalize has been called */
      if  ( ( comm->permanent == 1 ) && (MPIR_Has_been_initialized == 1) )
	  return MPIR_ERROR( comm, MPI_ERR_PERM_KEY,
			     "Error in MPI_COMM_FREE" );

      /* Remove it form the debuggers list of active communicators */
      MPIR_Comm_forget( comm );

#ifdef MPI_ADI2
        (void)MPID_CommFree( comm );
#else
        (void)MPID_Comm_free( comm->ADIctx, comm );
#endif

	/* Delete the virtual function table if it was allocated and
	 * is now no longer referenced. Ones which are statically
	 * set up have the ref count boosted beforehand, so they're
	 * never freed !
	 */
	if (comm->collops && --(comm->collops->ref_count) == 0)
	    FREE(comm->collops);
	comm->collops = NULL;

        DBG(FPRINTF(OUTFILE,"About to free context\n");fflush(OUTFILE);)
	/* Free the context used by this communicator */
	(void) MPIR_Context_dealloc ( comm, 1, comm->recv_context );
	
        DBG(FPRINTF(OUTFILE,"About to finish lock on comm\n");fflush(OUTFILE);)
	/* Free lock on collective comm, if it's not a self-reference */
	if ( comm->comm_coll != comm ) {
	  MPID_THREAD_LOCK_FINISH(comm->ADIctx,comm->comm_coll);
	    }

        DBG(FPRINTF(OUTFILE,"About to free coll comm\n");fflush(OUTFILE);)
	/* Free collective communicator (unless it refers back to myself) */
	if ( comm->comm_coll != comm )
	  MPI_Comm_free ( &(comm->comm_coll) );

	/* Put this after freeing the collective comm because it may have
	   incremented the ref count of the attribute tree */
        DBG(FPRINTF(OUTFILE,"About to free cache info\n");fflush(OUTFILE);)
	/* Free cache information */
	MPIR_Attr_free_tree ( comm );
	
        DBG(FPRINTF(OUTFILE,"About to free groups\n");fflush(OUTFILE);)
	/* Free groups */
	MPI_Group_free ( &(comm->group) );
	MPI_Group_free ( &(comm->local_group) );

	MPI_Errhandler_free( &(comm->error_handler) );

        /* Free off any name string that may be present */
        if (comm->comm_name)
	  {
	    FREE(comm->comm_name);
	    comm->comm_name = 0;
	  }

        DBG(FPRINTF(OUTFILE,"About to free comm structure\n");fflush(OUTFILE);)
	/* Free comm structure */
	MPIR_SET_COOKIE(comm,0);
	FREE( comm );
  }

  DBG(FPRINTF(OUTFILE,"About to set comm to comm_null\n");fflush(OUTFILE);)
  /* Set comm to null */
  *commp = MPI_COMM_NULL;
	
  return (mpi_errno);
}

