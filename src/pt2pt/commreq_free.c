/*
 *  $Id: commreq_free.c,v 1.15 1995/09/13 21:47:01 gropp Exp $
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
    int mpi_errno = MPI_SUCCESS;
    MPI_Request rq;

    if (MPIR_TEST_ARG(request) || MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,
			  "Error in MPI_REQUEST_FREE" );

    rq = *request;
    if (rq->chandle.persistent) {
	if (--rq->chandle.datatype->ref_count <= 1) {
	    MPIR_Type_free( &rq->chandle.datatype );
	    }
	/* Note: if this persistent request is ACTIVE, we must wait 
	   until it completes before freeing it; that free must take 
	   place within the wait */
	}

    if (rq->type == MPIR_SEND)
    {
    /*
        if (rq->shandle.mode == MPIR_MODE_BUFFERED) 
	    MPIR_BufferFreeReq( &rq->shandle );
     */
	MPID_Free_send_handle( rq->shandle.comm->ADIctx, 
			       &(rq->shandle.dev_shandle));
	MPIR_SET_COOKIE(&rq->shandle,0);
	MPIR_SBfree( MPIR_shandles, rq );
    }
    else if (rq->type == MPIR_RECV)
    {
	MPID_Free_recv_handle( rq->rhandle.comm->ADIctx, 
			       &(rq->rhandle.dev_rhandle));
	MPIR_SET_COOKIE(&rq->rhandle,0);
	MPIR_SBfree( MPIR_rhandles, rq );
    }
    else
	mpi_errno = MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INTERN, 
			    "MPI_Request_free:  bad request type" );

    *request = MPI_REQUEST_NULL;
    return mpi_errno;
}

