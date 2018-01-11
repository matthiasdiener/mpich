/*
 *  $Id: isend.c,v 1.24 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
#endif

/*@
    MPI_Isend - Begins a nonblocking send

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
int MPI_Isend( buf, count, datatype, dest, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              dest;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE *dtype_ptr;
    static char myname[] = "MPI_ISEND";
#ifdef MPI_ADI2
    int mpi_errno = MPI_SUCCESS;

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm_ptr,dest)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

    MPIR_ALLOC(*request,(MPI_Request) MPID_SendAlloc(),
	       comm_ptr,MPI_ERR_EXHAUSTED,myname );
    MPID_Request_init( (&(*request)->shandle), MPIR_SEND );

    /* Remember the send operation in case the user is interested while	
     * debugging. (This is a macro which may expand to nothing...)
     */
    MPIR_REMEMBER_SEND(&(*request)->shandle, buf, count, datatype, dest, tag, 
		       comm_ptr);

    if (dest == MPI_PROC_NULL) {
	(*request)->shandle.is_complete = 1;
	return MPI_SUCCESS;
    }
    /* This COULD test for the contiguous homogeneous case first .... */
    MPID_IsendDatatype( comm_ptr, buf, count, dtype_ptr, comm_ptr->local_rank, 
			tag, comm_ptr->send_context, 
			comm_ptr->lrank_to_grank[dest], 
			*request, &mpi_errno );
    if (mpi_errno) {
	/* We need to free the request ... */
	return MPIR_ERROR( comm_ptr, mpi_errno, myname );
    }
    return MPI_SUCCESS;
#else    
    int err;

    /* We'll let MPI_Send_init routine detect the errors */
    err = MPI_Send_init( buf, count, datatype, dest, tag, comm, request );
    if (err)
	return err;
    
    (*request)->shandle.persistent = 0;
    
    if (dest != MPI_PROC_NULL) {
	return MPI_Start( request );
	}

    /*
       This must create a completed request so that we can wait on it
     */
    MPID_Set_completed( comm_ptr->ADIctx, *request );
    (*request)->shandle.active     = 1;
    return MPI_SUCCESS;
#endif
}
