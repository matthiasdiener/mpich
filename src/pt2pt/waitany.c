/*
 *  $Id: waitany.c,v 1.17 1995/05/16 18:11:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: waitany.c,v 1.17 1995/05/16 18:11:31 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Waitany - Waits for any specified send or receive to complete

Input Parameters:
. count - list length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. index - index of handle for operation that completed (integer).  In the
range 0 to count-1.  In Fortran, the range is 1 to count.
. status - status object (Status) 
@*/
int MPI_Waitany(count, array_of_requests, index, status )
int         count;
MPI_Request array_of_requests[];
int         *index;
MPI_Status  *status;
{
    int i, mpi_errno = MPI_SUCCESS;
    MPI_Request request;

    *index = MPI_UNDEFINED;

    /* Check for the trivial case of nothing to do. */
    for (i=0; i<count; i++) {
	if (array_of_requests[i] && array_of_requests[i]->chandle.active) 
	    break;
	}
    if (i == count) return mpi_errno;

    while (1) {
	for (i = 0; i < count; i++)
	    {
	    /* Skip over null handle.  We need this for handles generated
	       when MPI_PROC_NULL is the source or destination of an 
	       operation */
	    request = array_of_requests[i];
	    if (!request || !request->chandle.active ) continue;

	    if (MPID_Test_request( MPID_Ctx( request ), request )) {
		*index = i;
		if ( request->type == MPIR_RECV ) {
		    MPID_Complete_recv( request->rhandle.comm->ADIctx,
				        &request->rhandle );
		    
		    status->MPI_SOURCE	   = request->rhandle.source;
		    status->MPI_TAG	   = request->rhandle.tag;
		    status->count	   = request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
		    if (request->rhandle.bufpos) 
			mpi_errno = MPIR_UnPackMessage( 
						   request->rhandle.bufadd, 
						   request->rhandle.count, 
						   request->rhandle.datatype, 
						   request->rhandle.source,
						   request, &status->count );
#endif
		    }
		else {
		  MPID_Complete_send( request->shandle.comm->ADIctx,
				      &request->shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		  if (request->shandle.bufpos && 
		      (mpi_errno = MPIR_SendBufferFree( request ))){
		    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
			       "Could not free allocated send buffer in MPI_WAITANY" );
		  }
#endif

		}

	        /* Spec requires, but... */
		if (!request->chandle.persistent) {
		    if (--request->chandle.datatype->ref_count <= 0) {
			MPIR_Type_free( &request->chandle.datatype );
			}
		    MPI_Request_free( &array_of_requests[i] ); 
		    /* array_of_requests[i]    = NULL; */
		    }
		else {
		    MPIR_RESET_PERSISTENT(request)
		    }
		return MPI_SUCCESS;
		}
	    }
	/* If nothing was ready, do a blocking wait on the device for 
	   anything to arrive; we'll then re-check the requests */
	MPID_Check_device( MPI_COMM_WORLD->ADIctx, MPID_BLOCKING );
	}
}

