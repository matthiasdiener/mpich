/*
 *  $Id: rsend.c,v 1.10 1994/10/24 22:02:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: rsend.c,v 1.10 1994/10/24 22:02:47 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Rsend - Basic ready send 

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

@*/
int MPI_Rsend( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int          errno = MPI_SUCCESS;
    MPIR_SHANDLE shandle;
    MPI_Request  request;
    MPI_Status   status;

    /* See send.c (MPI_Send) for a discussion of this routine. */
    if (dest != MPI_PROC_NULL)
    {
        request = (MPI_Request)&shandle;
        MPIR_Send_init( buf, count, datatype, dest, tag, comm, request, 
		        MPIR_MODE_READY, 0 );
	/* It is only at this point that we can detect a null input buffer */
	if (errno = MPIR_Send_setup(&request)) return errno;
	MPID_Blocking_send_ready( comm->ADIctx, &shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	/* If this request had to allocate a buffer to send from,
	   free it */
	if (shandle.bufpos && (errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( comm, errno, 
		       "Could not free allocated send buffer in MPI_RSEND" );
	    }
#endif

    }
    return errno;
}

