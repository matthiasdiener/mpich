/*
 *  $Id: testall.c,v 1.17 1995/05/16 18:11:00 gropp Exp $
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
    int i, found, mpi_errno;
    MPI_Request request;

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );
    found = 1;
    for (i = 0; i < count; i++)
	{
	request = array_of_requests[i];

	if ( !request || !request->chandle.active) {
	    /* See MPI Standard, 3.7 */
	    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
	    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
	    array_of_statuses[i].count	    = 0;
	    continue;
	    }
	else {
	    if (!MPID_Test_request( MPID_Ctx( request ), request)) {
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
	    if ( MPID_Test_request( MPID_Ctx( request ), request) ) {
		if ( request->type == MPIR_RECV ) {
		    array_of_statuses[i].MPI_SOURCE = request->rhandle.source;
		    array_of_statuses[i].MPI_TAG    = request->rhandle.tag;
		    array_of_statuses[i].count      = 
			request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
		    if (request->rhandle.bufpos) 
			if (mpi_errno = MPIR_UnPackMessage( 
					       request->rhandle.bufadd, 
					       request->rhandle.count, 
					       request->rhandle.datatype, 
					       request->rhandle.source,
					       request, 
   					       &array_of_statuses[i].count )) 
			    return mpi_errno;
#endif
		    }
		else {
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		    if (request->shandle.bufpos && 
			(mpi_errno = MPIR_SendBufferFree( request ))){
			MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
		       "Could not free allocated send buffer in MPI_TESTALL" );
			}
#endif
		    }
		if (!request->chandle.persistent) {
		    if (--request->chandle.datatype->ref_count <= 0) {
			MPIR_Type_free( &request->chandle.datatype );
			}
		    MPI_Request_free( &array_of_requests[i] );
		    /* Question: should we ALWAYS set to null? */
		    array_of_requests[i]    = NULL;
		    }
		else {
		    MPIR_RESET_PERSISTENT(request)
		    }
		}
	    else 
		found = 0;
	    }
	}
    *flag = found;	
    return MPI_SUCCESS;
}    
