/*
 *  $Id: start.c,v 1.24 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/* NOTES 
   We mark all sends and receives as non-blocking because that is safe here;
   unfortunately, we don't have enough information in the current send/recv
   handle to determine if we are blocking or not.
 */

/*@
    MPI_Start - Initiates a communication with a persistent request handle

Input Parameter:
. request - communication request (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST

@*/
int MPI_Start( request )
MPI_Request *request;
{
    int mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPI_START";
#ifdef MPI_ADI2
    MPIR_PSHANDLE *pshandle;
    MPIR_PRHANDLE *prhandle;
#else
    int req_type;
    MPIR_SHANDLE *shandle;
    MPIR_RHANDLE *rhandle;
#endif
    
    TR_PUSH(myname);

    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, myname );

#ifdef MPI_ADI2
    switch ((*request)->handle_type) {
    case MPIR_PERSISTENT_SEND:
	pshandle = &(*request)->persistent_shandle;
	if (pshandle->perm_dest == MPI_PROC_NULL) {
	    pshandle->active	          = 1;
	    pshandle->shandle.is_complete = 1;
	    TR_POP;
	    return MPI_SUCCESS;
	}
	if (pshandle->active) {
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_REQUEST, myname );
	}
	/* Since there are many send modes, we save the routine to
	   call in the handle */
	(*pshandle->send)( pshandle->perm_comm, pshandle->perm_buf, 
			  pshandle->perm_count, pshandle->perm_datatype, 
			  pshandle->perm_comm->local_rank, 
			  pshandle->perm_tag, 
			  pshandle->perm_comm->send_context, 
			  pshandle->perm_comm->lrank_to_grank[
			      pshandle->perm_dest], 
			  *request, &mpi_errno );
	if (mpi_errno) 
	    return MPIR_ERROR( pshandle->perm_comm, mpi_errno, myname );
	pshandle->active	 = 1;
	break;
    case MPIR_PERSISTENT_RECV:
	prhandle = &(*request)->persistent_rhandle;
	if (prhandle->perm_source == MPI_PROC_NULL) {
	    prhandle->active		   = 1;
	    prhandle->rhandle.is_complete  = 1;
	    prhandle->rhandle.s.MPI_TAG	   = MPI_ANY_TAG;
	    prhandle->rhandle.s.MPI_SOURCE = MPI_PROC_NULL;
	    prhandle->rhandle.s.count	   = 0;
	    TR_POP;
	    return MPI_SUCCESS;
	}
	if (prhandle->active) {
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_REQUEST, myname );
	}
	MPID_IrecvDatatype( prhandle->perm_comm, prhandle->perm_buf, 
			    prhandle->perm_count, prhandle->perm_datatype, 
			    prhandle->perm_source, prhandle->perm_tag, 
			    prhandle->perm_comm->recv_context, 
			    *request, &mpi_errno );
	if (mpi_errno) 
	    return MPIR_ERROR( prhandle->perm_comm, mpi_errno, myname );
	prhandle->active = 1;
	break;
    default:
	return MPIR_ERROR(MPIR_COMM_WORLD,MPI_ERR_REQUEST,myname );
    }
#else
    req_type = (*request)->type;

    if (req_type == MPIR_SEND) {
	/* I really should allow MPIR_SEND_SETUP_BUFFER here ... */
	/* Buffered send has special buffer setup requirements... */
	shandle = &(*request)->shandle;
	/* device will post the send */
	switch (shandle->mode) {
	    case MPIR_MODE_STANDARD:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD,myname);
	    MPIR_CALL(MPID_Post_send( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD, myname );
	    break;
	    case MPIR_MODE_SYNCHRONOUS:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD, myname ); 
	    MPIR_CALL(MPID_Post_send_sync( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD,myname );
	    break;
	    case MPIR_MODE_READY:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD,myname);
	    MPIR_CALL(MPID_Post_send_ready( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD,myname);
	    break;
	    case MPIR_MODE_BUFFERED:
	    MPIR_CALL(MPIR_DoBufferSend( shandle ),MPI_COMM_WORLD,myname);
	    break;
	    default:
	    return 
	       MPIR_ERROR( shandle->comm, MPI_ERR_INTERN,"Unknown send mode" );
	    }
	}
    else if (req_type == MPIR_RECV) {
	/* I really should allow MPIR_RECV_SETUP_BUFFER here ... */
	MPIR_CALL(MPIR_Receive_setup(request),MPI_COMM_WORLD,myname);
	/* device will handle queueing of MPIR recv handle in posted-recv 
	   queue         
	   */
	rhandle = &(*request)->rhandle;
	MPID_Set_recv_is_nonblocking( rhandle->comm->ADIctx, 
				     &(rhandle->dev_rhandle), 1 );
	MPIR_CALL(MPID_Post_recv( rhandle->comm->ADIctx, rhandle ),
		  MPI_COMM_WORLD,myname);
	}
    else
	mpi_errno = MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_INTERN,
		   "Bad request type in MPI_START");
#endif
    TR_POP;
    return mpi_errno;
}


