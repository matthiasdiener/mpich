/*
 *  $Id: rsend.c,v 1.16 1996/06/07 15:07:30 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


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

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK

@*/
int MPI_Rsend( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int          mpi_errno = MPI_SUCCESS;
#ifndef MPI_ADI2    
    MPIR_SHANDLE shandle;
    MPI_Request  request;
#endif

    if (MPIR_TEST_COMM(comm,comm) || 
	MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || 
	MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm,dest)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_RSEND" );


#ifdef MPI_ADI2
    if (dest == MPI_PROC_NULL) return MPI_SUCCESS;

    /* This COULD test for the contiguous homogeneous case first .... */
    MPID_RsendDatatype( comm, buf, count, datatype, comm->local_rank, tag, 
			comm->send_context, comm->lrank_to_grank[dest], 
			&mpi_errno );
    MPIR_RETURN(comm, mpi_errno, "Error in MPI_RSEND" );
#else
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
    shandle.datatype->ref_count--;
    }
    return mpi_errno;
#endif
}
