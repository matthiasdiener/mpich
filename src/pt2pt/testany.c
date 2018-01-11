/*
 *  $Id: testany.c,v 1.28 1996/01/11 18:30:53 gropp Exp $
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
. index - index of operation that completed, or 'MPI_UNDEFINED'  if none 
  completed (integer) 
. flag - true if one of the operations is complete (logical) 
. status - status object (Status) 

.N fortran
@*/
int MPI_Testany( count, array_of_requests, index, flag, status )
int         count;
MPI_Request array_of_requests[];
int         *index, *flag;
MPI_Status  *status;
{
    int i, found, mpi_errno = MPI_SUCCESS;
    MPI_Request request;
    *index = MPI_UNDEFINED;

    /* Check for the trivial case of nothing to do; check requests for
       validity */
    found	= 0;
    *flag	= 0;
    for (i=0; i<count; i++) {
	if (array_of_requests[i]) {
	    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,array_of_requests[i]))
		return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, 
				  "Error in MPI_TESTANY" );
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
		if (--request->chandle.datatype->ref_count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
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
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, "Error in MPI_TESTANY");
	}
    return mpi_errno;
}

