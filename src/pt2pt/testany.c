/*
 *  $Id: testany.c,v 1.34 1997/01/24 21:55:18 gropp Exp $
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
    MPI_Testany - Tests for completion of any previdously initiated 
                  communication

Input Parameters:
. count - list length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. index - index of operation that completed, or 'MPI_UNDEFINED'  if none 
  completed (integer) 
. flag - true if one of the operations is complete (logical) 
. status - status object (Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_IN_STATUS

@*/
int MPI_Testany( count, array_of_requests, index, flag, status )
int         count;
MPI_Request array_of_requests[];
int         *index, *flag;
MPI_Status  *status;
{
    int i, found, mpi_errno = MPI_SUCCESS;
    MPI_Request request;
    int nnull = 0;
    static char myname[] = "MPI_TESTANY";

    TR_PUSH(myname);
    *index = MPI_UNDEFINED;

#ifdef MPI_ADI2
    MPID_DeviceCheck( MPID_NOTBLOCKING );
    found = 0;
    for (i = 0; i < count && !found; i++) {
	/* Skip over null handles.  We need this for handles generated
	   when MPI_PROC_NULL is the source or destination of an 
	   operation */
	request = array_of_requests[i];

	if (!request) {
	    nnull ++;
	    continue;
	    }

	switch (request->handle_type) {
	case MPIR_SEND:
	    if (request->shandle.is_complete || 
		MPID_SendIcomplete( request, &mpi_errno )) {
		*index = i;
		MPIR_FORGET_SEND(&request->shandle);
		MPID_SendFree( request );
		array_of_requests[i] = 0;
		found = 1;
	    }
	    break;
	case MPIR_RECV:
	    if (request->rhandle.is_complete || 
		MPID_RecvIcomplete( request, (MPI_Status *)0, &mpi_errno )) {
		*index = i;
		*status = request->rhandle.s;
		MPID_RecvFree( request );
		array_of_requests[i] = 0;
		found = 1;
	    }
	    break;
	case MPIR_PERSISTENT_SEND:
	    if (!request->persistent_shandle.active) {
		nnull++;
	    }
	    else if (request->persistent_shandle.shandle.is_complete ||
		     MPID_SendIcomplete( request, &mpi_errno )) {
		*index = i;
		request->persistent_shandle.active = 0;
		found = 1;
	    }
	    break;
	case MPIR_PERSISTENT_RECV:
	    if (!request->persistent_rhandle.active) {
		nnull++;
	    }
	    else if (request->persistent_rhandle.rhandle.is_complete ||
		     MPID_RecvIcomplete( request, (MPI_Status *)0, 
					 &mpi_errno )) {
		*index = i;
		*status = request->persistent_rhandle.rhandle.s;
		request->persistent_rhandle.active = 0;
		found = 1;
	    }
	    break;
	}
    }
    if (nnull == count) {
	/* MPI Standard 1.1 requires an empty status in this case */
	status->MPI_TAG	   = MPI_ANY_TAG;
	status->MPI_SOURCE = MPI_ANY_SOURCE;
	status->MPI_ERROR  = MPI_SUCCESS;
	status->count	   = 0;
	*flag              = 1;
	TR_POP;
	return MPI_SUCCESS;
	}
    *flag = found;
    
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname );
	}
    TR_POP;
    return mpi_errno;
#else
    /* Check for the trivial case of nothing to do; check requests for
       validity */
    found	= 0;
    *flag	= 0;
    for (i=0; i<count; i++) {
	if (array_of_requests[i]) {
	    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,array_of_requests[i]))
		return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, myname );
	    if (array_of_requests[i]->chandle.active) found = 1;
	    }
	}
    
    /* If all handles are null or inactive, return flag = TRUE and an
       empty status (see MPI Standard Version 1.1) */
    
    if (!found) {
	/* MPI Standard 1.1 requires an empty status in this case */
	status->MPI_TAG	   = MPI_ANY_TAG;
	status->MPI_SOURCE = MPI_ANY_SOURCE;
	status->MPI_ERROR  = MPI_SUCCESS;
	status->count	   = 0;
	*flag              = 1;
	return MPI_SUCCESS;
	}

    MPID_Check_device( MPI_COMM_WORLD->ADIctx, 0 );
    found = 0;
    for (i = 0; i < count; i++)
    {
        request = array_of_requests[i];
	if ( !request  || !request->chandle.active ) continue;

	/* If Test request fails, we need to call the operation-specific
	   test routines to check the status of an operation.  See 
	   testall.c */
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
	
  	if (MPID_Test_request( MPID_Ctx( request ), request )) {
	    found         = 1;
	    *index        = i;
	    if ( request->type == MPIR_SEND ) {
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		if (request->shandle.bufpos && 
		      (mpi_errno = MPIR_SendBufferFree( request ))){
		    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
		      "Could not free allocated send buffer in MPI_TESTANY" );
		  }
#endif
	        }
	    else {
		if (request->rhandle.errval) {
		    mpi_errno = request->rhandle.errval;
		    }
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
						   request, 
   					           &status->count );
#endif
	    }
	    if (!request->chandle.persistent) {
		MPIR_Type_free( &request->chandle.datatype );
		/*
		if (--request->chandle.datatype->ref _count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
		    */
		MPI_Request_free( &array_of_requests[i] );
		array_of_requests[i]    = NULL;
		}
	    else {
		MPIR_RESET_PERSISTENT(request)
		}
	    break;
        }
    }
    *flag = found;
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname );
	}
    return mpi_errno;
#endif
}

