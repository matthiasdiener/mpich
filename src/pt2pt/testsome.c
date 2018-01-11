/*
 *  $Id: testsome.c,v 1.19 1996/01/11 18:30:57 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: testsome.c,v 1.19 1996/01/11 18:30:57 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Testsome - Tests for some given communications to complete

Input Parameters:
. incount - length of array_of_requests (integer) 
. array_of_requests - array of requests (array of handles) 

Output Parameters:
. outcount - number of completed requests (integer) 
. array_of_indices - array of indices of operations that 
completed (array of integers) 
. array_of_statuses - array of status objects for 
    operations that completed (array of Status) 

.N fortran
@*/
int MPI_Testsome( incount, array_of_requests, outcount, array_of_indices, 
		  array_of_statuses )
int         incount, *outcount, array_of_indices[];
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
{
    int i, mpi_errno = MPI_SUCCESS;
    int nfound = 0;
    int nnull  = 0;
    MPI_Request request;

    /* NOTE:
       This implementation will not work correctly if the device requires
       messages to be received in some particular order.  In that case, 
       this routine needs to try and complete the messages in ANY order.
       
       The same is true for testall.c .
     */
    MPID_Check_device( MPI_COMM_WORLD->ADIctx, MPID_NOTBLOCKING );
    for (i = 0; i < incount; i++)
	{
	/* Skip over null handles.  We need this for handles generated
	   when MPI_PROC_NULL is the source or destination of an 
	   operation */
	request = array_of_requests[i];

	if (!request || !request->chandle.active) {
	    nnull ++;
	    continue;
	    }

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
	    
	    array_of_indices[nfound] = i;
	    if ( request->type == MPIR_RECV )
		{
		if (request->rhandle.errval) {
		    array_of_statuses[nfound].MPI_ERROR  = 
			                              request->rhandle.errval;
		    mpi_errno = MPI_ERR_IN_STATUS;
		    }
		array_of_statuses[nfound].MPI_SOURCE = 
		    request->rhandle.source;
		array_of_statuses[nfound].MPI_TAG    = 
		    request->rhandle.tag;
		array_of_statuses[nfound].count  = 
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
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
		if (request->shandle.bufpos && 
		    (mpi_errno = MPIR_SendBufferFree( request ))){
		    MPIR_ERROR( MPI_COMM_NULL, mpi_errno, 
   	             "Could not free allocated send buffer in MPI_TESTSOME" );
		    }
#endif
		}
	    nfound++;
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
	    }
	}
    if (nnull == incount)
	*outcount = MPI_UNDEFINED;
    else
	*outcount = nfound;
    if (mpi_errno) {
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, "Error in MPI_TESTSOME");
	}
    return mpi_errno;
}
