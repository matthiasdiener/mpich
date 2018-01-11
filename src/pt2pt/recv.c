/*
 *  $Id: recv.c,v 1.21 1994/10/24 22:02:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: recv.c,v 1.21 1994/10/24 22:02:46 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
/*@
    MPI_Recv - Basic receive

Output Parameter:
. buf - initial address of receive buffer (choice) 
. status - status object (Status) 

Input Parameters:
. count - number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

@*/
int MPI_Recv( buf, count, datatype, source, tag, comm, status )
void             *buf;
int              count, source, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
{
    MPI_Request request;
    MPIR_RHANDLE rhandle;
    int         errno, is_available;

    /* 
       Because this is a very common routine, we show how it can be
       optimized to be run "inline"; In addition, this lets us exploit
       features in the ADI to simplify the execution of blocking receive
       calls.
     */
    if (source != MPI_PROC_NULL)
    {
        if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	    MPIR_TEST_DATATYPE(comm,datatype) || 
	    MPIR_TEST_RECV_TAG(comm,tag) ||
	    MPIR_TEST_RECV_RANK(comm,source)) 
	    return MPIR_ERROR(comm, errno, "Error in MPI_RECV" );

        request = (MPI_Request)&rhandle;
	request->type	    = MPIR_RECV;
	if (source == MPI_ANY_SOURCE)
	    rhandle.source  = source;
	else
	    rhandle.source  = comm->group->lrank_to_grank[source];
	rhandle.tag	    = tag;
	rhandle.contextid   = comm->recv_context;
	rhandle.comm        = comm;
	rhandle.datatype    = datatype;
	rhandle.bufadd	    = buf;
	rhandle.count	    = count;
	rhandle.completed   = MPIR_NO;
#ifdef MPID_HAS_HETERO
	rhandle.msgrep      = MPIR_MSGREP_UNKNOWN;
#endif
	MPID_Alloc_recv_handle(rhandle.comm->ADIctx, &(rhandle.dev_rhandle));

	if (errno = MPIR_Receive_setup(&request)) 
	    return MPIR_ERROR( comm, errno, "Error in MPI_RECV" );

	MPID_Blocking_recv( rhandle.comm->ADIctx, &rhandle );

	status->count	       = rhandle.totallen;
	status->MPI_SOURCE     = rhandle.source;
	status->MPI_TAG        = rhandle.tag;

#ifdef MPID_RETURN_PACKED
	if (rhandle.bufpos) 
	    errno = MPIR_UnPackMessage( buf, count, datatype, 
				        source, request );
#endif
    }
    else {
	/* See MPI standard section 3.11 */
	status->count	       = 0;
	status->MPI_SOURCE     = MPI_PROC_NULL;
	status->MPI_TAG        = MPI_ANY_TAG;
	}
    return MPI_SUCCESS;
}

