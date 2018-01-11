/*
 *  $Id: waitall.c,v 1.23 1994/10/24 22:02:57 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: waitall.c,v 1.23 1994/10/24 22:02:57 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Waitall - Waits for all given communications to complete

Input Parameters:
. count - lists length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameter:
. array_of_statuses - array of status objects (array of Status) 
@*/
int MPI_Waitall(count, array_of_requests, array_of_statuses )
int         count;
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
{
    int i;
    MPIR_BOOL completed;
    MPI_Request request;
    int errno;
    
    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       In particular, many systems need the sends completed before the
       receives.

       The same is true for testall.c .
     */
    /* Process all pending sends... */
    for (i = 0; i < count; i++)
    {
        /* Skip over null handles.  We need this for handles generated
           when MPI_PROC_NULL is the source or destination of an operation */
        request = array_of_requests[i];
        if (!request || !request->chandle.active ) continue;

	if ( request->type == MPIR_SEND ) {
	  MPID_Complete_send( request->shandle.comm->ADIctx, 
			      &request->shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	  if (request->shandle.bufpos && 
	      (errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( MPI_COMM_NULL, errno, 
		       "Could not free allocated send buffer in MPI_WAITALL" );
	  }
#endif	  
	  if (!request->chandle.persistent) {
	      if (--request->chandle.datatype->ref_count <= 0) {
		  MPIR_Type_free( &request->chandle.datatype );
		  }
	      MPI_Request_free( &array_of_requests[i] );
	      array_of_requests[i]    = NULL;
	      }
	  else {
	      request->chandle.active = 0;
	      request->chandle.completed = MPIR_NO;
	      MPID_Reuse_send_handle( request->shandle.comm->ADIctx, 
				      &request->shandle.dev_shandle );
	      }
	  }
	}
    /* Process all pending receives... */
    for (i = 0; i < count; i++)
    {
        /* Skip over null handles.  We need this for handles generated
           when MPI_PROC_NULL is the source or destination of an operation */
        request = array_of_requests[i];
        if (!request || !request->chandle.active ) continue;

	if ( request->type == MPIR_RECV ) {
	    if (request->chandle.completed == MPIR_NO) {
		MPID_Complete_recv( request->rhandle.comm->ADIctx, 
				    &request->rhandle );
		}
	    array_of_statuses[i].MPI_SOURCE	= request->rhandle.source;
	    array_of_statuses[i].MPI_TAG	= request->rhandle.tag;
	    array_of_statuses[i].count		= request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
	    if (request->rhandle.bufpos) 
		errno = MPIR_UnPackMessage( request->rhandle.bufadd, 
					    request->rhandle.count, 
					    request->rhandle.datatype, 
					    request->rhandle.source,
					    request );
#endif
	    if (!request->chandle.persistent) {
		if (--request->chandle.datatype->ref_count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
		MPI_Request_free( &array_of_requests[i] );
		array_of_requests[i]    = NULL;
		}
	    else {
		request->chandle.active = 0;
		request->chandle.completed = MPIR_NO;
		MPID_Reuse_recv_handle( request->rhandle.comm->ADIctx, 
				        &request->rhandle.dev_rhandle );
		}
	    }
    }
    return MPI_SUCCESS;
}







