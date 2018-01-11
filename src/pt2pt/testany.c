/*
 *  $Id: testany.c,v 1.19 1994/10/24 22:02:50 gropp Exp $
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
    int i, found, errno;
    MPI_Request request;
    *index = MPI_UNDEFINED;

    /* Check for the trivial case of nothing to do. */
    for (i=0; i<count; i++) {
	if (array_of_requests[i] && array_of_requests[i]->chandle.active) 
	    break;
	}
    if (i == count) return MPI_SUCCESS;

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );
    found = 0;
    for (i = 0; i < count; i++)
    {
        request = array_of_requests[i];
	if ( !request  || !request->chandle.active ) continue;

  	if ( request->chandle.completed == MPIR_YES) {
	    found         = 1;
	    *index        = i;
	    if ( request->type == MPIR_SEND ) {
		MPID_Complete_send( request->shandle.comm->ADIctx, 
				    &request->shandle );
	        }
	    else {
		MPID_Complete_recv( request->rhandle.comm->ADIctx, 
				    &request->rhandle );

		status->MPI_SOURCE     = request->rhandle.source;
		status->MPI_TAG	       = request->rhandle.tag;
		status->count	       = request->rhandle.totallen;
	    }
	    if (!request->chandle.persistent) {
		MPI_Request_free( &array_of_requests[i] );
		array_of_requests[i]    = NULL;
		}
	    else {
		request->chandle.active	        = 0;
		request->chandle.completed	= MPIR_NO;
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

