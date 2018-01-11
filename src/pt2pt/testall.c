/*
 *  $Id: testall.c,v 1.10 1998/07/01 19:56:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Testall - Tests for the completion of all previously initiated
    communications

Input Parameters:
+ count - lists length (integer) 
- array_of_requests - array of requests (array of handles) 

Output Parameters:
+ flag - (logical) 
- array_of_statuses - array of status objects (array of Status) 

Notes:
  'flag' is true only if all requests have completed.  Otherwise, flag is
  false and neither the 'array_of_requests' nor the 'array_of_statuses' is
  modified.

.N waitstatus

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
    static char myname[] = "MPI_TESTALL";

    TR_PUSH(myname);

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
	if (mpi_errno) {
	    MPIR_Set_Status_error_array( array_of_requests, count, i, 
					 mpi_errno, array_of_statuses );
	    mpi_errno = MPI_ERR_IN_STATUS;
	    TR_POP;
	    MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	}
    }
    *flag = (nready == count);
    /* Becausea request may have completed with an error (such as 
       MPI_ERR_TRUNCATE), we need to check here as well */
    if (nready == count) {
	for (i=0; i<count; i++) {
	    request = array_of_requests[i];
	    if (!request) {
		/* See MPI Standard, 3.7 */
		array_of_statuses[i].MPI_TAG	= MPI_ANY_TAG;
		array_of_statuses[i].MPI_SOURCE	= MPI_ANY_SOURCE;
		array_of_statuses[i].MPI_ERROR	= MPI_SUCCESS;
		array_of_statuses[i].count	= 0;
		continue;
	    }
	    switch (request->handle_type) {
	    case MPIR_SEND:
	        MPIR_FORGET_SEND( &request->shandle );
		MPID_SendFree( array_of_requests[i] );
		array_of_requests[i] = 0;
		break;
	    case MPIR_RECV:
		if (request->rhandle.s.MPI_ERROR) 
		    mpi_errno = request->rhandle.s.MPI_ERROR;
/*
		if (request->rhandle.s.MPI_ERROR && mpi_errno == MPI_SUCCESS) {
		    for (j=0; j<count; j++) {
			if (!array_of_requests[i] || 
			    array_of_requests[i].is_complete)
			    array_of_statuses[j].MPI_ERROR = MPI_SUCCESS;
			else
			    array_of_statuses[j].MPI_ERROR = MPI_ERR_PENDING;
		    }
		    mpi_errno = MPI_ERR_IN_STATUS;
		}
 */
		array_of_statuses[i] = request->rhandle.s;
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
		    mpi_errno = request->persistent_rhandle.rhandle.s.MPI_ERROR;
		    request->persistent_rhandle.active = 0;
		}
		else {
		    /* See MPI Standard, 3.7 */
		    /* Thanks to mechi@terra.co.il for this fix */
#ifdef FOO
		    if (request->persistent_rhandle.rhandle.s.MPI_TAG ==
			MPIR_MSG_CANCELLED) 
			array_of_statuses[i].MPI_TAG = MPIR_MSG_CANCELLED;
		    else
#endif
			array_of_statuses[i].MPI_TAG = MPI_ANY_TAG;
		    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
		    array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
		    array_of_statuses[i].count	    = 0;
		}
		break;
	    }
	    if (mpi_errno) {
		MPIR_Set_Status_error_array( array_of_requests, count, i, 
					     mpi_errno, array_of_statuses );
		mpi_errno = MPI_ERR_IN_STATUS;
		TR_POP;
		MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	    }
	}
    }
    TR_POP;
    MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
}
