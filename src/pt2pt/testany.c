/*
 *  $Id: testany.c,v 1.23 1995/03/07 16:18:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Testany - Tests for completion of any previdously initiated 
                  communication

Input Parameters:
. count - list length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. index - index of operation that completed, or MPI_UNDEFINED  if none 
  completed (integer) 
. flag - true if one of the operations is complete (logical) 
. status - status object (Status) 
@*/
int MPI_Testany( count, array_of_requests, index, flag, status )
int         count;
MPI_Request array_of_requests[];
int         *index, *flag;
MPI_Status  *status;
{
    int i, found, mpi_errno;
    MPI_Request request;
    *index = MPI_UNDEFINED;

    /* Check for the trivial case of nothing to do; check requests for
       validity */
    found = 0;
    *flag  = 0;
    for (i=0; i<count; i++) {
	if (array_of_requests[i]) {
	    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,array_of_requests[i]))
		return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, 
				  "Error in MPI_TESTANY" );
	    if (array_of_requests[i]->chandle.active) found = 1;
	    }
	}
    
    if (!found) return MPI_SUCCESS;

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );
    found = 0;
    for (i = 0; i < count; i++)
    {
        request = array_of_requests[i];
	if ( !request  || !request->chandle.active ) continue;

  	if (MPID_Test_request( MPID_Ctx( request ), request )) {
	    found         = 1;
	    *index        = i;
	    if ( request->type == MPIR_SEND ) {
		MPID_Complete_send( request->shandle.comm->ADIctx, 
				    &request->shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		if (request->shandle.bufpos && 
		      (mpi_errno = MPIR_SendBufferFree( request ))){
		    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
		      "Could not free allocated send buffer in MPI_TESTANY" );
		  }
#endif
	        }
	    else {
		MPID_Complete_recv( request->rhandle.comm->ADIctx, 
				    &request->rhandle );

		status->MPI_SOURCE     = request->rhandle.source;
		status->MPI_TAG	       = request->rhandle.tag;
		status->count	       = request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
		if (request->rhandle.bufpos) 
		    mpi_errno = MPIR_UnPackMessage( 
						   request->rhandle.bufadd, 
						   request->rhandle.count, 
						   request->rhandle.datatype, 
						   request->rhandle.source,
						   request );
#endif
	    }
	    if (!request->chandle.persistent) {
		if (--request->chandle.datatype->ref_count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
		MPI_Request_free( &array_of_requests[i] );
		array_of_requests[i]    = NULL;
		}
	    else {
		request->chandle.active	   = 0;
		MPID_Clr_completed( MPID_Ctx( request ), request );
		if (request->type == MPIR_RECV) {
		    MPID_Reuse_recv_handle( request->rhandle.comm->ADIctx,
					    &request->rhandle.dev_rhandle );
		    }
		else {
		    MPID_Reuse_send_handle( request->shandle.comm->ADIctx, 
					    &request->shandle.dev_shandle );
		    }
		}
	    break;
        }
    }
    *flag = found;
    return MPI_SUCCESS;
}

