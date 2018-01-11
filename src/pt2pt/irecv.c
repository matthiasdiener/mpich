/*
 *  $Id: irecv.c,v 1.21 1997/02/18 23:05:35 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
#endif

/*@
    MPI_Irecv - Begins a nonblocking receive

Input Parameters:
. buf - initial address of receive buffer (choice) 
. count - number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

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
    static char myname[] = "MPI_IRECV";
#ifdef MPI_ADI2
    int mpi_errno = MPI_SUCCESS;

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm_ptr,source)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

    MPIR_ALLOC(*request, (MPI_Request) MPID_RecvAlloc(),comm_ptr,
	       MPI_ERR_EXHAUSTED,myname);
    MPID_Request_init( (&(*request)->rhandle), MPIR_RECV );

    if (source == MPI_PROC_NULL) {
	(*request)->rhandle.s.MPI_TAG	 = MPI_ANY_TAG;
	(*request)->rhandle.s.MPI_SOURCE = MPI_PROC_NULL;
	(*request)->rhandle.s.count	 = 0;
	(*request)->rhandle.is_complete	 = 1;
	return MPI_SUCCESS;
    }
    MPID_IrecvDatatype( comm_ptr, buf, count, dtype_ptr, source, tag, 
			comm_ptr->recv_context, *request, &mpi_errno );
    if (mpi_errno) return MPIR_ERROR( comm_ptr, mpi_errno, myname );
    return MPI_SUCCESS;
#else
    int err;

    if (source != MPI_PROC_NULL)
    {
        /* We'll let this routine catch the errors */
        if ((err = 
	    MPI_Recv_init( buf, count, datatype, source, tag, comm, request )))
	    return err;
	(*request)->rhandle.persistent = 0;
	return MPI_Start( request );
    }
    else {
	if ((err = 
	    MPI_Recv_init( buf, count, datatype, source, tag, comm, request )))
	    return err;
	MPID_Set_completed( comm_ptr->ADIctx, *request );
	(*request)->rhandle.persistent = 0;
	(*request)->rhandle.active     = 1;
	(*request)->rhandle.source     = MPI_PROC_NULL;
	(*request)->rhandle.tag        = MPI_ANY_TAG;
	(*request)->rhandle.totallen   = 0;
	}
    return MPI_SUCCESS;
#endif
}
