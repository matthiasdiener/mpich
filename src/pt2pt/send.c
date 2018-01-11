/*
 *  $Id: send.c,v 1.27 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/* 
   Because this is a very common routine, we show how it can be
   optimized to be run "inline"; In addition, this lets us exploit
   features in the ADI to simplify the execution of blocking send 
   calls.
 */

/*@
    MPI_Send - Performs a basic send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

Notes:
This routine may block until the message is received.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK

.seealso: MPI_Isend, MPI_Bsend
@*/
int MPI_Send( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int          mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE *dtype_ptr;
    static char myname[] = "MPI_SEND";
#ifndef MPI_ADI2
    MPIR_SHANDLE shandle;
    MPI_Request  request;
#endif

    if (dest == MPI_PROC_NULL) 
	return mpi_errno;

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm_ptr,dest)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

#ifdef MPI_ADI2
    if (dest == MPI_PROC_NULL) return MPI_SUCCESS;

    /* This COULD test for the contiguous homogeneous case first .... */
    MPID_SendDatatype( comm_ptr, buf, count, dtype_ptr, comm_ptr->local_rank, 
		       tag, 
		       comm_ptr->send_context, comm_ptr->lrank_to_grank[dest], 
		       &mpi_errno );
    MPIR_RETURN(comm_ptr, mpi_errno, myname );
#else
    request = (MPI_Request)&shandle;

    request->type      = MPIR_SEND;
    shandle.contextid  = comm_ptr->send_context;
    shandle.dest       = comm_ptr->lrank_to_grank[dest];
    shandle.tag        = tag;
    
    MPID_Clr_completed(comm_ptr->ADIctx, request);

/*     MPIR_GET_REAL_DATATYPE(datatype) */
    shandle.datatype   = datatype;
/*    datatype->ref_ count++; */
    shandle.comm       = comm;

#ifdef MPID_HAS_HETERO
    shandle.msgrep	   = MPIR_MSGREP_SENDER;
#endif

    shandle.bufadd     = buf;
    shandle.count      = count;
    shandle.mode       = MPIR_MODE_STANDARD;
    shandle.lrank      = comm_ptr->local_rank;
    
/*
   The above is in 
    MPIR_Send_init( buf, count, datatype, dest, tag, comm, request, 
		   MPIR_MODE_STANDARD, 0 );
 */
    shandle.persistent   = 0;

    MPID_Alloc_send_handle(comm_ptr->ADIctx, &shandle.dev_shandle);
    MPID_Set_send_is_nonblocking(comm_ptr->ADIctx, &shandle.dev_shandle, 0);

    /* It is only at this point that we can detect a null input buffer.
       The next "routine" is a macro that sets mpi_errno */
    MPIR_SEND_SETUP_BUFFER( &request, shandle );
    if (mpi_errno) 
	return MPIR_ERROR(comm, mpi_errno, myname );
    MPID_Blocking_send( comm_ptr->ADIctx, &shandle );
#if defined(MPID_PACK_IN_ADVANCE) || defined(MPID_HAS_HETERO)
    /* If this request had to allocate a buffer to send from,
       free it */
    if (shandle.bufpos && (mpi_errno = MPIR_SendBufferFree( request ))){
	MPIR_ERROR( comm, mpi_errno, 
		   "Could not free allocated send buffer in MPI_SEND" );
	}
#endif
/*    shandle.datatype->ref_ count--; */
    return mpi_errno;
#endif
}


