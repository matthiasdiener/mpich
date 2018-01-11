/*
 *  $Id: commreq_free.c,v 1.12 1994/12/11 16:45:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Request_free - Frees a communication request object

Input Parameter:
. request - communication request (handle) 
@*/
int MPI_Request_free( request )
MPI_Request *request;
{
    int errno = MPI_SUCCESS;

    if (MPIR_TEST_ARG(request) || MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPI_COMM_WORLD,errno,"Error in MPI_REQUEST_FREE" );

    if ((*request)->chandle.persistent) {
	if (--(*request)->chandle.datatype->ref_count <= 1) {
	    MPIR_Type_free( &(*request)->chandle.datatype );
	    }
	/* Note: if this persistent request is ACTIVE, we must wait 
	   until it completes before freeing it; that free must take 
	   place within the wait */
	}

    if ((*request)->type == MPIR_SEND)
    {
        if ((*request)->shandle.mode == MPIR_MODE_BUFFERED) 
	    MPIR_BufferFreeReq( &(*request)->shandle );
	MPID_Free_send_handle( (*request)->shandle.comm->ADIctx, 
			       &((*request)->shandle.dev_shandle));
	MPIR_SET_COOKIE(&(*request)->shandle,0);
	MPIR_SBfree( MPIR_shandles, *request );
    }
    else if ((*request)->type == MPIR_RECV)
    {
	MPID_Free_recv_handle( (*request)->rhandle.comm->ADIctx, 
			       &((*request)->rhandle.dev_rhandle));
	MPIR_SET_COOKIE(&(*request)->rhandle,0);
	MPIR_SBfree( MPIR_rhandles, *request );
    }
    else
	errno = MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INTERN, 
			    "MPI_Request_free:  bad request type" );

    *request = MPI_REQUEST_NULL;
    return errno;
}

