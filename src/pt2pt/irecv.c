/*
 *  $Id: irecv.c,v 1.18 1996/06/07 15:07:30 gropp Exp $
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
#ifdef MPI_ADI2
    int mpi_errno = MPI_SUCCESS;

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_IRECV" );

    MPIR_ALLOC(*request, (MPI_Request) MPID_RecvAlloc(),comm,
	       MPI_ERR_EXHAUSTED,"Error in MPI_IRECV");
    MPID_Request_init( (&(*request)->rhandle), MPIR_RECV );
    
    if (source == MPI_PROC_NULL) {
	(*request)->rhandle.s.MPI_TAG	 = MPI_ANY_TAG;
	(*request)->rhandle.s.MPI_SOURCE = MPI_PROC_NULL;
	(*request)->rhandle.s.count	 = 0;
	(*request)->rhandle.is_complete	 = 1;
	return MPI_SUCCESS;
    }
    MPID_IrecvDatatype( comm, buf, count, datatype, source, tag, 
			comm->recv_context, *request, &mpi_errno );
    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, "Error in MPI_IRECV" );
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
	MPID_Set_completed( comm->ADIctx, *request );
	(*request)->rhandle.persistent = 0;
	(*request)->rhandle.active     = 1;
	}
    return MPI_SUCCESS;
#endif
}
