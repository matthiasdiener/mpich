/*
 *  $Id: irsend.c,v 1.10 1996/06/26 19:27:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
#endif

/*@
    MPI_Irsend - Starts a nonblocking ready send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 
Output Parameter:
. request - communication request (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
.N MPI_ERR_EXHAUSTED

@*/
int MPI_Irsend( buf, count, datatype, dest, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              dest;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
#ifdef MPI_ADI2
    int mpi_errno;

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm,dest)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_IRSEND" );

    MPIR_ALLOC(*request,(MPI_Request) MPID_SendAlloc(),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_IRSEND" );
    MPID_Request_init( (&(*request)->shandle), MPIR_SEND );

    MPIR_REMEMBER_SEND((&(*request)->shandle), buf, count, datatype, dest, tag, comm);

    if (dest == MPI_PROC_NULL) {
	(*request)->shandle.is_complete = 1;
	return MPI_SUCCESS;
    }
    /* This COULD test for the contiguous homogeneous case first .... */
    MPID_IrsendDatatype( comm, buf, count, datatype, comm->local_rank, tag, 
			 comm->send_context, comm->lrank_to_grank[dest], 
			 *request, &mpi_errno );
    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, "Error in MPI_IRSEND" );
    return MPI_SUCCESS;
#else
    int mpi_errno;
    if (dest != MPI_PROC_NULL)
    {
        /* We'll let MPI_Rsend_init find the errors */
        if ((mpi_errno = 
        MPI_Rsend_init( buf, count, datatype, dest, tag, comm, request )))
	    return mpi_errno;
	(*request)->shandle.persistent = 0;
	return MPI_Start( request );
    }
    else {
	if ((mpi_errno = 
	    MPI_Send_init( buf, count, datatype, dest, tag, comm, request )))
	    return mpi_errno;
	MPID_Set_completed( comm->ADIctx, *request );
	(*request)->shandle.persistent = 0;
	(*request)->shandle.active     = 1;
	}
    return MPI_SUCCESS;
#endif
}
