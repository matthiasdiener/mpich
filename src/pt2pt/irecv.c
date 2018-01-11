/*
 *  $Id: irecv.c,v 1.3 1998/04/28 21:46:53 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Irecv - Begins a nonblocking receive

Input Parameters:
+ buf - initial address of receive buffer (choice) 
. count - number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 

.N fortran
@*/
int MPI_Irecv( buf, count, datatype, source, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              source;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE *dtype_ptr;
    MPIR_RHANDLE *rhandle;
    static char myname[] = "MPI_IRECV";
    int mpi_errno = MPI_SUCCESS;

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm_ptr,source)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

    MPIR_ALLOCFN(rhandle,MPID_RecvAlloc,comm_ptr,
	       MPI_ERR_EXHAUSTED,myname);
    MPID_Request_init( rhandle, MPIR_RECV );
    *request = (MPI_Request) rhandle;

    if (source == MPI_PROC_NULL) {
	rhandle->s.MPI_TAG    = MPI_ANY_TAG;
	rhandle->s.MPI_SOURCE = MPI_PROC_NULL;
	rhandle->s.count      = 0;
	rhandle->is_complete  = 1;
	return MPI_SUCCESS;
    }
    MPID_IrecvDatatype( comm_ptr, buf, count, dtype_ptr, source, tag, 
			comm_ptr->recv_context, *request, &mpi_errno );
    if (mpi_errno) return MPIR_ERROR( comm_ptr, mpi_errno, myname );
    return MPI_SUCCESS;
}

