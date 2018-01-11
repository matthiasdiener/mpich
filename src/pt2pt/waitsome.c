/*
 *  $Id: waitsome.c,v 1.28 1997/01/24 21:55:18 gropp Exp $
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
    MPI_Waitsome - Waits for some given communications to complete

Input Parameters:
. incount - length of array_of_requests (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. outcount - number of completed requests (integer) 
. array_of_indices - array of indices of operations that 
completed (array of integers) 
. array_of_statuses - array of status objects for 
    operations that completed (array of Status) 

Notes:
  The array of indicies are in the range '0' to 'incount - 1' for C and 
in the range '1' to 'incount' for Fortran.  

Null requests are ignored; if all requests are null, then the routine
returns with 'outcount' set to 'MPI_UNDEFINED'.

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
@*/
int MPI_Waitsome( incount, array_of_requests, outcount, array_of_indices, 
		  array_of_statuses )
int         incount, *outcount, array_of_indices[];
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
{
    int i, mpi_errno = MPI_SUCCESS;
    MPI_Request request;
    int nnull, mpi_lerr;
    int nfound = 0;
    static char myname[] = "MPI_WAITSOME";

    TR_PUSH(myname);
#ifdef MPI_ADI2
    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       
       The same is true for waitall.c .
     */
    nnull = 0;
    while (nfound == 0 && nnull < incount ) {
	MPID_DeviceCheck( MPID_NOTBLOCKING );
	nnull = 0;
	for (i = 0; i < incount; i++) {
	    /* Skip over null handles.  We need this for handles generated
	       when MPI_PROC_NULL is the source or destination of an 
	       operation */
	    request = array_of_requests[i];

	    if (!request) {/*  || !request->chandle.active) { */
		nnull ++;
		continue;
	    }

	    mpi_lerr = 0;
	    switch (request->handle_type) {
	    case MPIR_SEND:
		if (request->shandle.is_complete || 
		    MPID_SendIcomplete( request, &mpi_lerr )) {
		    array_of_indices[nfound] = i;
		    array_of_statuses[nfound].MPI_ERROR = mpi_lerr;
		    if (mpi_lerr) mpi_errno = MPI_ERR_IN_STATUS;
		    MPIR_FORGET_SEND( &request->shandle );
		    MPID_SendFree( request );
		    array_of_requests[i] = 0;
		    nfound++;
		}
		break;
	    case MPIR_RECV:
		if (request->rhandle.is_complete || 
		    MPID_RecvIcomplete( request, (MPI_Status *)0, &mpi_lerr )) {
		    array_of_indices[nfound]  = i;
		    array_of_statuses[nfound] = request->rhandle.s;
		    if (request->rhandle.s.MPI_ERROR) 
			mpi_errno = MPI_ERR_IN_STATUS;
		    MPID_RecvFree( request );
		    array_of_requests[i] = 0;
		    nfound++;
		}
		break;
	    case MPIR_PERSISTENT_SEND:
		if (!request->persistent_shandle.active) {
		    nnull++;
		}
		else if (request->persistent_shandle.shandle.is_complete ||
			 MPID_SendIcomplete( request, &mpi_lerr )) {
		    array_of_indices[nfound] = i;
		    array_of_statuses[nfound].MPI_ERROR = mpi_lerr;
		    if (mpi_lerr) mpi_errno = MPI_ERR_IN_STATUS;
		    request->persistent_shandle.active = 0;
		    nfound++;
		}
		break;
	    case MPIR_PERSISTENT_RECV:
		if (!request->persistent_rhandle.active) {
		    nnull++;
		}
		else if (request->persistent_rhandle.rhandle.is_complete ||
			 MPID_RecvIcomplete( request, (MPI_Status *)0, 
					     &mpi_lerr )) {
		    array_of_indices[nfound] = i;
		    array_of_statuses[nfound] = 
			request->persistent_rhandle.rhandle.s;
		    if (mpi_lerr) mpi_errno = MPI_ERR_IN_STATUS;
		    request->persistent_rhandle.active = 0;
		    nfound++;
		}
		break;
	    }
	}
    }
    if (nnull == incount)
	*outcount = MPI_UNDEFINED;
    else
	*outcount = nfound;
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname);
	}
    TR_POP;
    return mpi_errno;
#else
    /* Check for the trivial case of nothing to do. */
    for (i=0; i<incount; i++) {
	if (array_of_requests[i] && array_of_requests[i]->chandle.active) 
	    break;
	}
    if (i == incount) {
	*outcount = MPI_UNDEFINED;
	return MPI_SUCCESS;
	}

    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       
       The same is true for testall.c .
     */
    while (nfound == 0) {
	for (i = 0; i < incount; i++)
	    {
	    /* Skip over null handles.  We need this for handles generated
	       when MPI_PROC_NULL is the source or destination of an 
	       operation */
	    request = array_of_requests[i];

	    if (!request || !request->chandle.active ) continue;

	    if (MPID_Test_request( MPID_Ctx( request ), request )) {
		array_of_indices[nfound] = i;
		if ( request->type == MPIR_RECV )
		    {
		    MPID_Complete_recv( request->rhandle.comm->ADIctx, 
				        &request->rhandle );
		    if (request->rhandle.errval) {
			array_of_statuses[nfound].MPI_ERROR  = 
			    request->rhandle.errval;
			mpi_errno = MPI_ERR_IN_STATUS;
			}
		    array_of_statuses[nfound].MPI_SOURCE = 
			request->rhandle.source;
		    array_of_statuses[nfound].MPI_TAG    = 
			request->rhandle.tag;
		    array_of_statuses[nfound].count      = 
			request->rhandle.totallen;
		    
#ifdef MPID_RETURN_PACKED
		    if (request->rhandle.bufpos) 
			mpi_errno = MPIR_UnPackMessage( 
						   request->rhandle.bufadd, 
						   request->rhandle.count, 
						   request->rhandle.datatype, 
						   request->rhandle.source,
						   request, 
					    &array_of_statuses[nfound].count );
#endif
		    }
		else {
		  MPID_Complete_send( request->shandle.comm->ADIctx, 
				      &request->shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		  if (request->shandle.bufpos && 
		      (mpi_errno = MPIR_SendBufferFree( request ))){
		    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
			       "Could not free allocated send buffer in MPI_WAITSOME" );
		    }
#endif
		  }
		nfound++;
		/* Spec requires, but... */
		if (!request->chandle.persistent) {
		    MPIR_Type_free( &request->chandle.datatype );
/*		    if (--request->chandle.datatype->ref_ count <= 0) {
			MPIR_Type_free( &request->chandle.datatype );
			}*/
		    MPI_Request_free( &array_of_requests[i] ); 
		    /* array_of_requests[i]    = NULL; */
		    }
		else {
		    MPIR_RESET_PERSISTENT(request)
		    }
		}
	    }
	/* See the comments in MPI_WAITANY (waitany.c).  This isn't 
	   a safe call */
	/*
	if (nfound == 0) 
	    MPID_Check_device( MPIR_COMM_WORLD->ADIctx, MPID_BLOCKING );
	     */
	    MPID_Check_device( MPIR_COMM_WORLD->ADIctx, MPID_NOTBLOCKING );

	}
    *outcount = nfound;
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname );
	}
    return mpi_errno;
#endif
}
