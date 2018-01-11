/*
 *  $Id: testsome.c,v 1.11 1994/10/24 22:02:51 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: testsome.c,v 1.11 1994/10/24 22:02:51 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Testsome - Tests for some given communications to complete

Input Parameters:
. incount - length of array_of_requests (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. outcount - number of completed requests (integer) 
. array_of_indices - array of indices of operations that 
completed (array of integers) 
. array_of_statuses - array of status objects for 
    operations that completed (array of Status) 
@*/
int MPI_Testsome( incount, array_of_requests, outcount, array_of_indices, 
		  array_of_statuses )
int         incount, *outcount, array_of_indices[];
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
{
    int i;
    MPIR_BOOL completed;
    int nfound = 0;
    MPI_Request request;

    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       
       The same is true for testall.c .
     */
    MPID_Check_device( MPI_COMM_WORLD->ADIctx, MPID_NOTBLOCKING );
    for (i = 0; i < incount; i++)
	{
	/* Skip over null handles.  We need this for handles generated
	   when MPI_PROC_NULL is the source or destination of an 
	   operation */
	request = array_of_requests[i];

	if (!request || !request->chandle.active) continue;

	if (request->chandle.completed) {
	    
	    array_of_indices[nfound] = i;
	    if ( request->type == MPIR_RECV )
		{
		MPID_Complete_recv( request->rhandle.comm->ADIctx,
				    &request->rhandle );
		
		array_of_statuses[nfound].MPI_SOURCE = 
		    request->rhandle.source;
		array_of_statuses[nfound].MPI_TAG    = 
		    request->rhandle.tag;
		array_of_statuses[nfound].count  = 
		    request->rhandle.totallen;
		}
	    else 
		MPID_Complete_send( request->shandle.comm->ADIctx,
				    &request->shandle );
	    nfound++;
	    if (!request->chandle.persistent) {
		MPI_Request_free( &array_of_requests[i] ); 
		array_of_requests[i]    = NULL;
		}
	    else {
		request->chandle.active	= 0;
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
	    }
	}
    *outcount = nfound;
    return MPI_SUCCESS;
}
