/*
 *  $Id: start.c,v 1.20 1996/01/08 19:47:30 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: start.c,v 1.20 1996/01/08 19:47:30 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

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
@*/
int MPI_Start( request )
MPI_Request *request;
{
    int req_type;
    int mpi_errno = MPI_SUCCESS;
    MPIR_SHANDLE *shandle;
    MPIR_RHANDLE *rhandle;

    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, "Error in MPI_START" );

    req_type = (*request)->type;

    if (req_type == MPIR_SEND) {
	/* I really should allow MPIR_SEND_SETUP_BUFFER here ... */
	/* Buffered send has special buffer setup requirements... */
	shandle = &(*request)->shandle;
	/* device will post the send */
	switch (shandle->mode) {
	    case MPIR_MODE_STANDARD:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD,
		      "Error in MPI_START" );
	    MPIR_CALL(MPID_Post_send( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD, "Error in MPI_START" );
	    break;
	    case MPIR_MODE_SYNCHRONOUS:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD, 
				  "Error in MPI_START" );
	    MPIR_CALL(MPID_Post_send_sync( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD,"Error in MPI_START" );
	    break;
	    case MPIR_MODE_READY:
	    MPIR_CALL(MPIR_Send_setup(request),MPI_COMM_WORLD,
		      "Error in MPI_START");
	    MPIR_CALL(MPID_Post_send_ready( shandle->comm->ADIctx, shandle ),
		      MPI_COMM_WORLD,"Error in MPI_START");
	    break;
	    case MPIR_MODE_BUFFERED:
	    MPIR_CALL(MPIR_DoBufferSend( shandle ),MPI_COMM_WORLD,
		      "Error in MPI_START");
	    break;
	    default:
	    return 
	       MPIR_ERROR( shandle->comm, MPI_ERR_INTERN,"Unknown send mode" );
	    }
	}
    else if (req_type == MPIR_RECV) {
	/* I really should allow MPIR_RECV_SETUP_BUFFER here ... */
	MPIR_CALL(MPIR_Receive_setup(request),MPI_COMM_WORLD,
		  "Error in MPI_START");
	/* device will handle queueing of MPIR recv handle in posted-recv 
	   queue         
	   */
	rhandle = &(*request)->rhandle;
	MPID_Set_recv_is_nonblocking( rhandle->comm->ADIctx, 
				     &(rhandle->dev_rhandle), 1 );
	MPIR_CALL(MPID_Post_recv( rhandle->comm->ADIctx, rhandle ),
		  MPI_COMM_WORLD,"Error in MPI_START"); 
	}
    else
	mpi_errno = MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_INTERN,
		   "Bad request type in MPI_START");
    return mpi_errno;
}


