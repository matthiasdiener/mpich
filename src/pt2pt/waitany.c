/*
 *  $Id: waitany.c,v 1.6 1998/07/01 19:56:17 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Waitany - Waits for any specified send or receive to complete

Input Parameters:
+ count - list length (integer) 
- array_of_requests - array of requests (array of handles) 

Output Parameters:
+ index - index of handle for operation that completed (integer).  In the
range '0' to 'count-1'.  In Fortran, the range is '1' to 'count'.
- status - status object (Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
@*/
int MPI_Waitany(count, array_of_requests, index, status )
int         count;
MPI_Request array_of_requests[];
int         *index;
MPI_Status  *status;
{
    int i, mpi_errno = MPI_SUCCESS;
    int done;
    MPI_Request request;
    static char myname[] = "MPI_WAITANY";

    TR_PUSH(myname);
    *index = MPI_UNDEFINED;

    /* Check for all requests either null or inactive persistent */
    for (i=0; i < count; i++) {
	request = array_of_requests[i];
	if (!request) continue;
	if (request->handle_type == MPIR_PERSISTENT_SEND) {
	    if (request->persistent_shandle.active) break;
	}
	else if (request->handle_type == MPIR_PERSISTENT_RECV) {
	    if (request->persistent_rhandle.active) break;
	}
	else 
	    break;
    }
    if (i == count) {
	/* MPI Standard 1.1 requires an empty status in this case */
 	status->MPI_TAG	   = MPI_ANY_TAG;
	status->MPI_SOURCE = MPI_ANY_SOURCE;
	status->MPI_ERROR  = MPI_SUCCESS;
	MPID_ZERO_STATUS_COUNT(status);
        *index             = -1;
	TR_POP;
	return mpi_errno;
	}
    done = 0;
    while (!done) {
	for (i=0; !done && i<count; i++) {
	    request = array_of_requests[i];
	    if (!request) continue;
	    switch (request->handle_type) {
	    case MPIR_SEND:
		if (MPID_SendIcomplete( request, &mpi_errno )) {
		    if (mpi_errno) 
			MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
		    MPIR_FORGET_SEND( &request->shandle );
		    MPID_SendFree( array_of_requests[i] );
		    *index = i;
		    array_of_requests[i] = 0;
		    done = 1;
		}
		break;
	    case MPIR_RECV:
		if (MPID_RecvIcomplete( request, status, &mpi_errno )) {
		    if (mpi_errno) 
			MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
		    MPID_RecvFree( array_of_requests[i] );
		    *index = i;
		    array_of_requests[i] = 0;
		    done = 1;
		}
		break;
	    case MPIR_PERSISTENT_SEND:
		if (request->persistent_shandle.active) {
		    if (MPID_SendIcomplete( request, &mpi_errno )) {
			if (mpi_errno) 
			    MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
			request->persistent_shandle.active = 0;
			*index = i;
			done = 1;
		    }
		}
		break;
	    case MPIR_PERSISTENT_RECV:
		if (request->persistent_rhandle.active) {
		    if (MPID_RecvIcomplete( request, status, &mpi_errno )) {
			if (mpi_errno) 
			    MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
			request->persistent_rhandle.active = 0;
			*index = i;
			done   = 1;
		    }
		}
		break;
	    }
	}
	if (!done) {
	    /* Do a NON blocking check */
	    MPID_DeviceCheck( MPID_NOTBLOCKING );
	}
	else 
	    break;
    }
    TR_POP;
    return mpi_errno;
}
