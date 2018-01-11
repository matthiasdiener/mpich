/*
 *  $Id: commreq_free.c,v 1.19 1996/07/17 18:04:00 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
/* pt2pt for MPIR_Type_free */
#include "mpipt2pt.h"
#else
#include "mpisys.h"
#endif

/*@
    MPI_Request_free - Frees a communication request object

Input Parameter:
. request - communication request (handle) 

Notes:
This routine is normally used to free persistent requests created with 
either 'MPI_Recv_init' or 'MPI_Send_init' and friends.  However, it can be 
used to free a request created with 'MPI_Irecv' or 'MPI_Isend' and friends;
in that case the use can not use the test/wait routines on the request.

It `is` permitted to free an active request.  However, once freed, you can not
use the request in a wait or test routine (e.g., 'MPI_Wait').

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG

.see also: MPI_Isend, MPI_Irecv, MPI_Issend, MPI_Ibsend, MPI_Irsend,
MPI_Recv_init, MPI_Send_init, MPI_Ssend_init, MPI_Rsend_init, MPI_Wait,
MPI_Test, MPI_Waitall, MPI_Waitany, MPI_Waitsome, MPI_Testall, MPI_Testany,
MPI_Testsome
@*/
int MPI_Request_free( request )
MPI_Request *request;
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Request rq;

    if (MPIR_TEST_ARG(request) || MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,
			  "Error in MPI_REQUEST_FREE" );
#ifdef MPI_ADI2
    rq = *request;
    switch( rq->handle_type) {
    case MPIR_SEND:
	if (rq->shandle.is_complete) {
	    MPIR_FORGET_SEND( &rq->shandle );
	    MPID_SendFree( rq );
	}
	else {
	    MPID_Request_free( rq );
	}
	*request = 0;
	break;
    case MPIR_RECV:
	if (rq->rhandle.is_complete) {
	    MPID_RecvFree( rq );
	}
	else {
	    MPID_Request_free( rq );
	}
	*request = 0;
	break;
    case MPIR_PERSISTENT_SEND:
	if (!rq->persistent_shandle.active) {
	    /* Must also free references to perm objects */
	    MPIR_Type_free( &rq->persistent_shandle.perm_datatype );
	    MPI_Comm_free( &rq->persistent_shandle.perm_comm );
	    MPID_PSendFree( rq );
	}
	else {
	    MPID_Request_free( rq );
	}
	*request = 0;
	break;
    case MPIR_PERSISTENT_RECV:
	if (!rq->persistent_rhandle.active) {
	    MPIR_Type_free( &rq->persistent_rhandle.perm_datatype );
	    MPI_Comm_free( &rq->persistent_rhandle.perm_comm );
	    MPID_PRecvFree( rq );
	}
	else {
	    MPID_Request_free( rq );
	}
	*request = 0;
	break;
    }

#else
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
#endif
    return mpi_errno;
}

