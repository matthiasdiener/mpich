/*
 *  $Id: rsend.c,v 1.19 1997/01/07 01:45:29 gropp Exp $
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
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    static char myname[] = "MPI_RSEND";
#ifndef MPI_ADI2    
    MPIR_SHANDLE shandle;
    MPI_Request  request;
#endif

    TR_PUSH(myname);
    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) ||MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm_ptr,dest)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

#ifdef MPI_ADI2
    if (dest == MPI_PROC_NULL) return MPI_SUCCESS;

    /* This COULD test for the contiguous homogeneous case first .... */
    MPID_RsendDatatype( comm_ptr, buf, count, dtype_ptr, comm_ptr->local_rank,
			tag, comm_ptr->send_context, 
			comm_ptr->lrank_to_grank[dest], &mpi_errno );
    MPIR_RETURN(comm_ptr, mpi_errno, myname );
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
	MPID_Blocking_send_ready( comm_ptr->ADIctx, &shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
	/* If this request had to allocate a buffer to send from,
	   free it */
	if (shandle.bufpos && (mpi_errno = MPIR_SendBufferFree( request ))){
	    MPIR_ERROR( comm, mpi_errno, 
		       "Could not free allocated send buffer in MPI_RSEND" );
	    }
#endif
    MPIR_Type_free( &shandle.datatype );
    /* shandle.datatype->ref_count --; */
    }
    return mpi_errno;
#endif
}
