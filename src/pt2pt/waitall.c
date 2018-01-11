/*
 *  $Id: waitall.c,v 1.8 1998/04/28 21:47:35 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Waitall - Waits for all given communications to complete

Input Parameters:
+ count - lists length (integer) 
- array_of_requests - array of requests (array of handles) 

Output Parameter:
. array_of_statuses - array of status objects (array of Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
.N MPI_ERR_IN_STATUS
.N MPI_ERR_PENDING
@*/
int MPI_Waitall(count, array_of_requests, array_of_statuses )
int         count;
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
{
    int i;
    MPI_Request request;
    int mpi_errno = MPI_SUCCESS, rc = MPI_SUCCESS;
    static char myname[] = "MPI_WAITALL";

    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       In particular, many systems need the sends completed before the
       receives.

       The same is true for testall.c .
     */
    /*
     * Note on the ordering of operations:
     * While this routine requests the device to complete the requests in 
     * a certain order (the order in which the requests are listed in the
     * array), requests must be completed in the order ISSUED.  The
     * test examples/test/pt2pt/waitall checks for this.
     */
    /* Process all pending sends... */
    for (i = 0; i < count; i++)
    {
        /* Skip over null handles.  We need this for handles generated
           when MPI_PROC_NULL is the source or destination of an operation */
        request = array_of_requests[i];
	/*FPRINTF( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request ); */
        if (!request) {
	    /* See MPI Standard, 3.7 */
	    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
	    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
	    array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
	    array_of_statuses[i].count	    = 0;
	    continue;
	    }

	if ( request->handle_type == MPIR_SEND ) {
	    rc = MPI_SUCCESS;
	    MPID_SendComplete( request, &rc );
	    if (rc) {
		MPIR_Set_Status_error_array( array_of_requests, count, i, 
					     rc, array_of_statuses );
		mpi_errno = MPI_ERR_IN_STATUS;
		MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	    }
	    MPIR_FORGET_SEND( &request->shandle );
	    MPID_SendFree( array_of_requests[i] );
	    array_of_requests[i]    = 0;
	}
	else if (request->handle_type == MPIR_PERSISTENT_SEND) {
	    if (!request->persistent_shandle.active) {
                /* See MPI Standard, 3.7 */
		array_of_statuses[i].MPI_TAG	= MPI_ANY_TAG;
		array_of_statuses[i].MPI_SOURCE	= MPI_ANY_SOURCE;
		array_of_statuses[i].MPI_ERROR	= MPI_SUCCESS;
		array_of_statuses[i].count	= 0;
		continue;
	    }
	    rc = MPI_SUCCESS;
	    MPID_SendComplete( request, &rc );
	    if (rc) {
		MPIR_Set_Status_error_array( array_of_requests, count, i, 
					     rc, array_of_statuses );
		mpi_errno = MPI_ERR_IN_STATUS;
		MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	    }
	    request->persistent_shandle.active = 0;
	}
    }

    /* Process all pending receives... */
    for (i = 0; i < count; i++)
    {
        /* Skip over null handles.  We need this for handles generated
           when MPI_PROC_NULL is the source or destination of an operation
           Note that the send loop has already set the status array entries */
        request = array_of_requests[i];
	/*FPRINTF( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request );*/
        if (!request) continue;

	if ( request->handle_type == MPIR_RECV ) {
	    /*FPRINTF( stderr, "[%d] receive request %d\n", MPIR_tid, i );*/
	    /* Old code does test first */
	    rc = MPI_SUCCESS;
	    MPID_RecvComplete( request, &array_of_statuses[i], &rc );
	    if (rc) {
		MPIR_Set_Status_error_array( array_of_requests, count, i, 
					     rc, array_of_statuses );
		mpi_errno = MPI_ERR_IN_STATUS;
		MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	    }
	    MPID_RecvFree( array_of_requests[i] );
	    array_of_requests[i] = 0;
	}
	else if (request->handle_type == MPIR_PERSISTENT_RECV) {
	    if (!request->persistent_rhandle.active) {
#ifdef FOO
		/* Thanks to mechi@terra.co.il for this fix */
		if (request->persistent_rhandle.rhandle.s.MPI_TAG ==
		    MPIR_MSG_CANCELLED) 
		    array_of_statuses[i].MPI_TAG = MPIR_MSG_CANCELLED;
		else
#endif
		    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
		array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
		array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
		array_of_statuses[i].count	    = 0;
		continue;
	    }
	    rc = MPI_SUCCESS;
	    MPID_RecvComplete( request, &array_of_statuses[i], &rc );
	    if (rc) {
		MPIR_Set_Status_error_array( array_of_requests, count, i, 
					     rc, array_of_statuses );
		mpi_errno = MPI_ERR_IN_STATUS;
		MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, myname );
	    }
	    request->persistent_rhandle.active = 0;
	}
    }
    
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname);
	}
    return mpi_errno;
}
