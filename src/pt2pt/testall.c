/*
 *  $Id: testall.c,v 1.10 1994/10/24 22:02:50 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Testall - Tests for the completion of all previously initiated
    communications

Input Parameters:
. count - lists length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. flag - (logical) 
. array_of_statuses - array of status objects (array of Status) 
@*/
int MPI_Testall( count, array_of_requests, flag, array_of_statuses )
int        count;
MPI_Request array_of_requests[];
int        *flag;
MPI_Status *array_of_statuses;
{
    int i, found;
    MPI_Status status;
    MPI_Request request;

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );
    found = 1;
    for (i = 0; i < count; i++)
	{
	request = array_of_requests[i];

	if ( request != NULL && 
	     request->chandle.active) {
	    if (request->chandle.completed != MPIR_YES) {
		/* Try to complete the send or receive */
		if (request->type == MPIR_SEND) {
		    if (MPID_Test_send( request->shandle.comm->ADIctx, 
				        &request->shandle )) {
			MPID_Complete_send( request->shandle.comm->ADIctx, 
					    &request->shandle );
			}
		    }
		else {
		    if (MPID_Test_recv( request->rhandle.comm->ADIctx, 
				        &request->rhandle )) {
			MPID_Complete_recv( request->rhandle.comm->ADIctx, 
					    &request->rhandle );
			}
		    }
		}
	    if ( request->chandle.completed == MPIR_YES) {
		if ( request->type == MPIR_RECV ) {
		    array_of_statuses[i].MPI_SOURCE = request->rhandle.source;
		    array_of_statuses[i].MPI_TAG    = request->rhandle.tag;
		    array_of_statuses[i].count      = 
			request->rhandle.totallen;
		    }
		if (!request->chandle.persistent) {
		    MPI_Request_free( &array_of_requests[i] );
		    /* Question: should we ALWAYS set to null? */
		    array_of_requests[i]    = NULL;
		    }
		else {
		    request->chandle.active    = 0;
		    request->chandle.completed = MPIR_NO;
		    if (request->type == MPIR_RECV) {
			MPID_Reuse_recv_handle(request->rhandle.comm->ADIctx, 
					      &request->rhandle.dev_rhandle );
			}
		    else {
			MPID_Reuse_send_handle(request->shandle.comm->ADIctx,
					      &request->shandle.dev_shandle );
			}
		    }
		}
	    else 
		found = 0;
	    }
	}
    *flag = found;	
    return MPI_SUCCESS;
}    
