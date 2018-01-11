/*
 *  $Id: start.c,v 1.16 1994/12/15 16:44:55 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: start.c,v 1.16 1994/12/15 16:44:55 gropp Exp $";
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

@*/
int MPI_Start( request )
MPI_Request *request;
{
    int is_available;
    int mpi_errno = MPI_SUCCESS;
    MPIR_SHANDLE *shandle;
    MPIR_RHANDLE *rhandle;

    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, "Error in MPI_START" );

    /* See also send.c */
    if ((*request)->type == MPIR_SEND)
    {
      if (mpi_errno = MPIR_Send_setup(request)) return mpi_errno;
      shandle = &(*request)->shandle;
      /* device will post the send */
      if ((*request)->shandle.mode == MPIR_MODE_SYNCHRONOUS)
	  MPID_Post_send_sync( shandle->comm->ADIctx, shandle );
      else if ((*request)->shandle.mode == MPIR_MODE_READY)
	  MPID_Post_send_ready( shandle->comm->ADIctx, shandle );
      else if ((*request)->shandle.mode == MPIR_MODE_BUFFERED) {
	  /* Need to prepare the buffer before sending */
	  MPIR_PrepareBuffer( shandle );
	  MPID_Post_send( shandle->comm->ADIctx, shandle );
	  }
      else
	  MPID_Post_send( shandle->comm->ADIctx, shandle );
    }
    else if ((*request)->type == MPIR_RECV)
    {
      if (mpi_errno = MPIR_Receive_setup(request)) return mpi_errno;
      /* device will handle queueing of MPIR recv handle in posted-recv 
	 queue         
       */
      rhandle = &(*request)->rhandle;
      MPID_Set_recv_is_nonblocking( rhandle->comm->ADIctx, 
				      &(rhandle->dev_rhandle), 1 );
      MPID_Post_recv( rhandle->comm->ADIctx, rhandle, &(is_available) ); 
    }
    else
	mpi_errno = MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_INTERN,
		   "Bad request type in MPI_START");
    return mpi_errno;
}


