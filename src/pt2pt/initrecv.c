/*
 *  $Id
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: initrecv.c,v 1.8 1995/01/03 19:43:15 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/* This initializes the fields of a request structure */

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
    request->rhandle.contextid    = comm->recv_context;
    request->rhandle.comm         = comm;
    request->rhandle.datatype     = datatype;
    datatype->ref_count++;
    request->rhandle.bufadd       = buf;
    request->rhandle.count        = count;
    request->rhandle.completed    = MPIR_NO;
#ifdef MPID_HAS_HETERO
    request->rhandle.msgrep	  = MPIR_MSGREP_UNKNOWN;
#endif
    MPID_Alloc_recv_handle( comm->ADIctx, &((request)->rhandle.dev_rhandle));
    MPID_Set_recv_is_nonblocking( comm->ADIctx, 
				 &((request)->rhandle.dev_rhandle), 
				 nonblocking );
    return MPI_SUCCESS;
}
