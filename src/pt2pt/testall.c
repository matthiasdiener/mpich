/*
 *  $Id: testall.c,v 1.24 1996/06/26 19:27:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
#else
#include "mpisys.h"
#endif

/*@
    MPI_Testall - Tests for the completion of all previously initiated
    communications

Input Parameters:
. count - lists length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. flag - (logical) 
. array_of_statuses - array of status objects (array of Status) 

Notes:
  'flag' is true only if all requests have completed.  Otherwise, flag is
  false and neither the 'array_of_requests' nor the 'array_of_statuses' is
  modified.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_IN_STATUS

@*/
int MPI_Testall( count, array_of_requests, flag, array_of_statuses )
int        count;
MPI_Request array_of_requests[];
int        *flag;
MPI_Status *array_of_statuses;
{
    int i, mpi_errno = MPI_SUCCESS;
    MPI_Request request;
    int nready;

#ifdef MPI_ADI2
    MPID_DeviceCheck( MPID_NOTBLOCKING );
  /* It is a good thing that the receive requests contain the status object!
     We need this to save the status information in the case where not
     all of the requests have completed.

     Note that this routine forces some changes on the ADI test routines.
     It must be possible to test a completed request multiple times;
     once the "is_complete" field is set, the data must be saved until
     the request is explicitly freed.  That is, unlike the MPI tests,
     the ADI tests must be nondestructive.
   */
    nready = 0;
    for (i = 0; i < count; i++ ) {
	request = array_of_requests[i];
	if (!request) {
	    nready ++;
	    continue;
	}
	switch (request->handle_type) {
	case MPIR_SEND:
	    if (!request->shandle.is_complete) {
		if (MPID_SendIcomplete( request, &mpi_errno ))
		    nready++;
	    }
	    else nready++;
	    break;
	case MPIR_RECV:
	    if (!request->rhandle.is_complete) {
		if (MPID_RecvIcomplete( request, (MPI_Status *)0, &mpi_errno ))
		    nready++;
	    }
	    else nready++;
	    break;
	case MPIR_PERSISTENT_SEND:
	    if (request->persistent_shandle.active &&
		!request->persistent_shandle.shandle.is_complete) {
		if (MPID_SendIcomplete( request, &mpi_errno ))
		    nready++;
	    }
	    else nready++;
	    break;
	case MPIR_PERSISTENT_RECV:
	    if (request->persistent_rhandle.active &&
		!request->persistent_rhandle.rhandle.is_complete) {
		if (MPID_RecvIcomplete( request, (MPI_Status *)0, &mpi_errno ))
		    nready++;
	    }
	    else nready++;
	    break;
	}
    }
    *flag = (nready == count);
    if (nready == count) {
	for (i=0; i<count; i++) {
	    request = array_of_requests[i];
	    if (!request) {
		/* See MPI Standard, 3.7 */
		array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
		array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
		array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
		array_of_statuses[i].count	    = 0;
		continue;
	    }
	    switch (request->handle_type) {
	    case MPIR_SEND:
	        MPIR_FORGET_SEND( &request->shandle );
		MPID_SendFree( array_of_requests[i] );
		array_of_requests[i] = 0;
		break;
	    case MPIR_RECV:
		array_of_statuses[i] = request->rhandle.s;
		if (request->rhandle.s.MPI_ERROR) {
		    mpi_errno = MPI_ERR_IN_STATUS;
		}
		MPID_RecvFree( array_of_requests[i] );
		array_of_requests[i] = 0;
		break;
	    case MPIR_PERSISTENT_SEND:
		request->persistent_shandle.active = 0;
		break;
	    case MPIR_PERSISTENT_RECV:
		if (request->persistent_rhandle.active) {
		    array_of_statuses[i] = 
			request->persistent_rhandle.rhandle.s;
		    request->persistent_rhandle.active = 0;
		}
		else {
		    /* See MPI Standard, 3.7 */
		    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
		    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
		    array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
		    array_of_statuses[i].count	    = 0;
		}
		break;
	    }
	}
    }
#else
    int mpi_lerr;

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );

    /* This test must go in two phases: First, see if all are ready.
       If not, then exit with flag false.  Otherwise, process them
       ALL and return with flag true. 
     */
    nready = 0;
    for (i = 0; i < count; i++)	{
	request = array_of_requests[i];

	if ( !request || !request->chandle.active) {
	    nready++;
	    continue;
	    }
	if (!MPID_Test_request( MPID_Ctx( request ), request)) {
	    /* Try to complete the send or receive */
	    if (request->type == MPIR_SEND) {
		if (MPID_Test_send( request->shandle.comm->ADIctx, 
				   &request->shandle )) 
		    nready++;
		}
	    else {
		if (MPID_Test_recv( request->rhandle.comm->ADIctx, 
				    &request->rhandle )) 
		    nready++;
		}
	    }
	else 
	    nready++;
	}
    
    /* If we exited early, then we're done (false return) */
    if (nready != count) {
	*flag = 0;
	return MPI_SUCCESS;
	}

    *flag = 1;

    /* Now gather information on the completed requests */
    for (i = 0; i < count; i++)	{
	request = array_of_requests[i];

	if ( !request || !request->chandle.active) {
	    /* See MPI Standard, 3.7 */
	    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
	    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
	    array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
	    array_of_statuses[i].count	    = 0;
	    continue;
	    }
	/* We know that we can complete the operation. 
           We don't need to use the Test_send/recv, but it doesn't hurt. */
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
		    mpi_lerr = MPID_Complete_recv( 
					    request->rhandle.comm->ADIctx, 
					    &request->rhandle );
		    if (mpi_lerr) {
			mpi_errno = MPI_ERR_IN_STATUS;
			array_of_statuses[i].MPI_ERROR = mpi_lerr;
			}
		    }
		}
	    }
	/* At this point, the request is complete */
	if ( request->type == MPIR_RECV ) {
	    if (request->rhandle.errval) {
		array_of_statuses[i].MPI_ERROR  = 
		    request->rhandle.errval;
		mpi_errno = MPI_ERR_IN_STATUS;
		}
	    array_of_statuses[i].MPI_SOURCE = request->rhandle.source;
	    array_of_statuses[i].MPI_TAG    = request->rhandle.tag;
	    array_of_statuses[i].count      = 
		request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
	    if (request->rhandle.bufpos) 
		if ((mpi_lerr = MPIR_UnPackMessage( 
						  request->rhandle.bufadd, 
						  request->rhandle.count, 
						  request->rhandle.datatype, 
						  request->rhandle.source,
						  request, 
					      &array_of_statuses[i].count ))) {
		    array_of_statuses[i].MPI_ERROR = mpi_lerr;
		    mpi_errno = MPI_ERR_IN_STATUS;
		    }
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

    if (mpi_errno) {
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, "Error in MPI_TESTALL");
	}
#endif
    return mpi_errno;
}
