/*
 *  $Id: waitall.c,v 1.29 1995/05/16 18:11:27 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: waitall.c,v 1.29 1995/05/16 18:11:27 gropp Exp $";
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
    MPI_Request request;
    int mpi_errno;
    
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
	/*fprintf( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request ); */
        if (!request || !request->chandle.active ) {
	    /* See MPI Standard, 3.7 */
	    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
	    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
	    array_of_statuses[i].count	    = 0;
	    continue;
	    }

	if ( request->type == MPIR_SEND ) {
	  MPID_Complete_send( request->shandle.comm->ADIctx, 
			      &request->shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	  if (request->shandle.bufpos && 
	      (mpi_errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
		       "Could not free allocated send buffer in MPI_WAITALL" );
	  }
#endif	  
	  if (!request->chandle.persistent) {
	      if (--request->chandle.datatype->ref_count <= 0) {
		  MPIR_Type_free( &request->chandle.datatype );
		  }
	      /*fprintf( stderr, "[%d] freeing request %d\n", MPIR_tid, i );*/
	      MPI_Request_free( &array_of_requests[i] );
	      /* sets array_of_requests[i]    = NULL; */
	      }
	  else {
	      MPIR_RESET_PERSISTENT_SEND(request)
	      }
	  }
	}
    /* Process all pending receives... */
    for (i = 0; i < count; i++)
    {
        /* Skip over null handles.  We need this for handles generated
           when MPI_PROC_NULL is the source or destination of an operation
           Note that the send loop has already set the status array entries */
        request = array_of_requests[i];
	/*fprintf( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request );*/
        if (!request || !request->chandle.active ) continue;

	if ( request->type == MPIR_RECV ) {
	    /*fprintf( stderr, "[%d] receive request %d\n", MPIR_tid, i );*/
	    if (! MPID_Test_request( MPID_Ctx( request ), request )) {
		MPID_Complete_recv( request->rhandle.comm->ADIctx, 
				    &request->rhandle );
		/*fprintf( stderr, "[%d] did complete receive\n", MPIR_tid );*/
		}
	    array_of_statuses[i].MPI_SOURCE	= request->rhandle.source;
	    array_of_statuses[i].MPI_TAG	= request->rhandle.tag;
	    array_of_statuses[i].count		= request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
	    if (request->rhandle.bufpos) {
		/*fprintf( stderr, "[%d] doing unpack from %d\n", 
			 MPIR_tid, array_of_statuses[i].MPI_SOURCE );*/
		mpi_errno = MPIR_UnPackMessage( request->rhandle.bufadd, 
					       request->rhandle.count, 
					       request->rhandle.datatype, 
					       request->rhandle.source,
					       request, 
   					       &array_of_statuses[i].count );
		}
#endif
	    if (!request->chandle.persistent) {
		if (--request->chandle.datatype->ref_count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
		/*fprintf( stderr, "[%d] freeing request %d\n", MPIR_tid, i );*/
		MPI_Request_free( &array_of_requests[i] );
		/* sets array_of_requests[i]    = NULL; */
		}
	    else {
		MPIR_RESET_PERSISTENT_RECV(request);
		}
	    }
    }
    return MPI_SUCCESS;
}







