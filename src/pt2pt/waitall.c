/*
 *  $Id: waitall.c,v 1.39 1997/01/24 21:55:18 gropp Exp $
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
    MPI_Waitall - Waits for all given communications to complete

Input Parameters:
. count - lists length (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameter:
. array_of_statuses - array of status objects (array of Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
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

#ifdef MPI_ADI2
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
		array_of_statuses[i].MPI_ERROR = rc;
		mpi_errno = MPI_ERR_IN_STATUS;
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
		array_of_statuses[i].MPI_ERROR = rc;
		mpi_errno = MPI_ERR_IN_STATUS;
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
		/* array_of_statuses[i].MPI_ERROR = rc; */
		mpi_errno = MPI_ERR_IN_STATUS;
	    }
	    MPID_RecvFree( array_of_requests[i] );
	    array_of_requests[i] = 0;
	}
	else if (request->handle_type == MPIR_PERSISTENT_RECV) {
	    if (!request->persistent_rhandle.active) {
		array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
		array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
		array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
		array_of_statuses[i].count	    = 0;
		continue;
	    }
	    rc = MPI_SUCCESS;
	    MPID_RecvComplete( request, &array_of_statuses[i], &rc );
	    if (rc) mpi_errno = MPI_ERR_IN_STATUS;
	    request->persistent_rhandle.active = 0;
	}
    }
    
#else    
    int mpi_lerr;
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
	/*FPRINTF( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request ); */
        if (!request || !request->chandle.active ) {
	    /* See MPI Standard, 3.7 */
	    array_of_statuses[i].MPI_TAG    = MPI_ANY_TAG;
	    array_of_statuses[i].MPI_SOURCE = MPI_ANY_SOURCE;
	    array_of_statuses[i].MPI_ERROR  = MPI_SUCCESS;
	    array_of_statuses[i].count	    = 0;
	    continue;
	    }

	if ( request->type == MPIR_SEND ) {
	    MPIR_CALL(MPID_Complete_send( request->shandle.comm->ADIctx, 
					  &request->shandle ),
		      MPIR_COMM_WORLD,"Could not complete send in MPI_WAITALL");
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	  if (request->shandle.bufpos && 
	      (mpi_errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
		       "Could not free allocated send buffer in MPI_WAITALL" );
	  }
#endif	  
	  if (!request->chandle.persistent) {
	      MPIR_Type_free( &request->chandle.datatype );
/*
	      if (--request->chandle.datatype->ref_ count <= 0) {
		  MPIR_Type_free( &request->chandle.datatype );
		  }
		  */
	      /*FPRINTF( stderr, "[%d] freeing request %d\n", MPIR_tid, i );*/
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
	/*FPRINTF( stderr, "[%d] processing request %d = %x\n", MPIR_tid, i, 
		 request );*/
        if (!request || !request->chandle.active ) continue;

	if ( request->type == MPIR_RECV ) {
	    /*FPRINTF( stderr, "[%d] receive request %d\n", MPIR_tid, i );*/
	    if (! MPID_Test_request( MPID_Ctx( request ), request )) {
		/* Need to trap error messages, particularly truncated 
		   data */
		mpi_lerr = MPID_Complete_recv( request->rhandle.comm->ADIctx, 
					        &request->rhandle );
		if (mpi_lerr) {
		    array_of_statuses[i].MPI_ERROR = mpi_lerr;
		    mpi_errno = MPI_ERR_IN_STATUS;
		    }
		/*FPRINTF( stderr, "[%d] did complete receive\n", MPIR_tid );*/
		}
	    if (request->rhandle.errval) {
		array_of_statuses[i].MPI_ERROR  = request->rhandle.errval;
		mpi_errno = MPI_ERR_IN_STATUS;
		}
	    array_of_statuses[i].MPI_SOURCE	= request->rhandle.source;
	    array_of_statuses[i].MPI_TAG	= request->rhandle.tag;
	    array_of_statuses[i].count		= request->rhandle.totallen;
#ifdef MPID_RETURN_PACKED
	    if (request->rhandle.bufpos) {
		/*FPRINTF( stderr, "[%d] doing unpack from %d\n", 
			 MPIR_tid, array_of_statuses[i].MPI_SOURCE );*/
		mpi_lerr = MPIR_UnPackMessage( request->rhandle.bufadd, 
					       request->rhandle.count, 
					       request->rhandle.datatype, 
					       request->rhandle.source,
					       request, 
   					       &array_of_statuses[i].count );
		if (mpi_lerr) {
		    array_of_statuses[i].MPI_ERROR = mpi_lerr;
		    mpi_errno = MPI_ERR_IN_STATUS;
		    }
		}
#endif
	    if (!request->chandle.persistent) {
		MPIR_Type_free( &request->chandle.datatype );
/*
		if (--request->chandle.datatype->ref_ count <= 0) {
		    MPIR_Type_free( &request->chandle.datatype );
		    }
		    */
		/*FPRINTF( stderr, "[%d] freeing request %d\n", MPIR_tid, i );*/
		MPI_Request_free( &array_of_requests[i] );
		/* sets array_of_requests[i]    = NULL; */
		}
	    else {
		MPIR_RESET_PERSISTENT_RECV(request);
		}
	    }
    }
#endif
    if (mpi_errno) {
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname);
	}
    return mpi_errno;
}
