/*
 *  $Id: testany.c,v 1.7 1998/04/28 21:47:20 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Testany - Tests for completion of any previdously initiated 
                  communication

Input Parameters:
+ count - list length (integer) 
- array_of_requests - array of requests (array of handles) 

Output Parameters:
+ index - index of operation that completed, or 'MPI_UNDEFINED'  if none 
  completed (integer) 
. flag - true if one of the operations is complete (logical) 
- status - status object (Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
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
	MPID_ZERO_STATUS_COUNT(status);
	*flag              = 1;
        *index             = -1;
	TR_POP;
	return MPI_SUCCESS;
	}
    *flag = found;
    
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname );
	}
    TR_POP;
    return mpi_errno;
}

