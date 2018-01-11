/*
 *  $Id
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/* This initializes the fields of a request structure */
#ifndef MPI_ADI2
#include "mpisys.h"
int MPIR_Send_init( buf, count, datatype, dest, tag, comm, request, mode,
		    nonblocking )
void          *buf;
int           count;
MPI_Datatype  datatype;
int           dest;
int           tag;
MPI_Comm      comm;
MPI_Request   request;
MPIR_Mode     mode;
int           nonblocking;
{
    struct MPIR_DATATYPE *dtype_ptr;
    int    mpi_errno;

    /* For better cache behavior, we order these as they are found in the
       actual structure */

    request->type                 = MPIR_SEND;
    request->shandle.contextid    = comm->send_context;
    if (dest == MPI_PROC_NULL)
	request->shandle.dest     = dest;
    else
	request->shandle.dest     = comm->group->lrank_to_grank[dest];
    request->shandle.tag          = tag;
    MPID_Clr_completed( comm->ADIctx, request );
/*    MPIR_GET_REAL_DATATYPE(datatype) */
    request->shandle.datatype     = datatype;
    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,MPI_COMM_WORLD,"MPI_SSEND_INIT");
    MPIR_REF_INCR(dtype_ptr);
    request->shandle.comm         = comm;
#ifdef MPID_HAS_HETERO
    request->shandle.msgrep	  = MPIR_MSGREP_SENDER;
#endif

    request->shandle.bufadd       = buf;
    request->shandle.count        = count;

    request->shandle.mode         = mode;
    request->shandle.lrank        = comm->local_group->local_rank;

    MPID_Alloc_send_handle( comm->ADIctx, &((request)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm->ADIctx, 
				 &((request)->shandle.dev_shandle), 
				 nonblocking );
return MPI_SUCCESS;
}
#endif
