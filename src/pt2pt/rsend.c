/*
 *  $Id: rsend.c,v 1.13 1995/05/09 18:10:38 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: rsend.c,v 1.13 1995/05/09 18:10:38 gropp Exp $";
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
    int          mpi_errno = MPI_SUCCESS;
    MPIR_SHANDLE shandle;
    MPI_Request  request;

    /* See send.c (MPI_Send) for a discussion of this routine. */
    if (dest != MPI_PROC_NULL)
    {
        request = (MPI_Request)&shandle;
        MPIR_Send_init( buf, count, datatype, dest, tag, comm, request, 
		        MPIR_MODE_READY, 0 );
	/* It is only at this point that we can detect a null input buffer.
	   The next "routine" is a macro that sets mpi_errno */
	MPIR_SEND_SETUP_BUFFER( &request, shandle );
	if (mpi_errno) 
	    return mpi_errno;
	MPID_Blocking_send_ready( comm->ADIctx, &shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	/* If this request had to allocate a buffer to send from,
	   free it */
	if (shandle.bufpos && (mpi_errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( comm, mpi_errno, 
		       "Could not free allocated send buffer in MPI_RSEND" );
	    }
#endif

    }
    return mpi_errno;
}

