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
int MPIR_Recv_init( buf, count, datatype, source, tag, comm, request, 
		    nonblocking )
void          *buf;
int           count;
MPI_Datatype  datatype;
int           source;
int           tag;
MPI_Comm      comm;
MPI_Request   request;
int           nonblocking;
{
    request->type                 = MPIR_RECV;
    request->rhandle.source       = source;
    request->rhandle.tag          = tag;
    request->rhandle.errval       = MPI_SUCCESS;
    request->rhandle.contextid    = comm->recv_context;
    request->rhandle.comm         = comm;
    MPIR_GET_REAL_DATATYPE(datatype)
    request->rhandle.datatype     = datatype;
    datatype->ref_count++;
    request->rhandle.bufadd       = buf;
    request->rhandle.count        = count;
    MPID_Clr_completed( comm->ADIctx, request );
#ifdef MPID_HAS_HETERO
    request->rhandle.msgrep	  = MPIR_MSGREP_UNKNOWN;
#endif
    MPID_Alloc_recv_handle( comm->ADIctx, &((request)->rhandle.dev_rhandle));
    MPID_Set_recv_is_nonblocking( comm->ADIctx, 
				 &((request)->rhandle.dev_rhandle), 
				 nonblocking );
    return MPI_SUCCESS;
}
#endif
