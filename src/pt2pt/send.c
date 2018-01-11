/*
 *  $Id: send.c,v 1.20 1994/12/15 16:58:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: send.c,v 1.20 1994/12/15 16:58:24 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Send - Basic send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

@*/
int MPI_Send( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int          mpi_errno = MPI_SUCCESS;
    MPIR_SHANDLE shandle;
    MPI_Request  request;

    /* 
       Because this is a very common routine, we show how it can be
       optimized to be run "inline"; In addition, this lets us exploit
       features in the ADI to simplify the execution of blocking send 
       calls.
     */
    if (dest != MPI_PROC_NULL)
    {
        if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	    MPIR_TEST_DATATYPE(comm,datatype) || 
	    MPIR_TEST_SEND_TAG(comm,tag) ||
	    MPIR_TEST_SEND_RANK(comm,dest)) 
	    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_SEND" );

        request = (MPI_Request)&shandle;
        MPIR_Send_init( buf, count, datatype, dest, tag, comm, request, 
		        MPIR_MODE_STANDARD, 0 );
	shandle.persistent   = 0;
	/* It is only at this point that we can detect a null input buffer */
	if (mpi_errno = MPIR_Send_setup(&request)) 
	    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_SEND" );
	MPID_Blocking_send( comm->ADIctx, &shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	/* If this request had to allocate a buffer to send from,
	   free it */
	if (shandle.bufpos && (mpi_errno = MPIR_SendBufferFree( request ))){
	  MPIR_ERROR( comm, mpi_errno, 
		     "Could not free allocated send buffer in MPI_SEND" );
	}
#endif
	shandle.datatype->ref_count--;
    }
    return mpi_errno;
}


