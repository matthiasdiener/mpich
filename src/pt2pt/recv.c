/*
 *  $Id: recv.c,v 1.31 1995/06/21 15:37:58 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: recv.c,v 1.31 1995/06/21 15:37:58 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
/*@
    MPI_Recv - Basic receive

Output Parameter:
. buf - initial address of receive buffer (choice) 
. status - status object (Status) 

Input Parameters:
. count - maximum number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

Notes:
The 'count' argument indicates the maximum length of a message; the actual 
number can be determined with MPI_Get_count.  

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
    int         mpi_errno = MPI_SUCCESS;

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
	    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_RECV" );

        request = (MPI_Request)&rhandle;
	request->type	   = MPIR_RECV;
#ifdef MPID_NEEDS_WORLD_SRC_INDICES
        rhandle.source = (source >= 0) ? (comm->lrank_to_grank[source])
                                         : source;
#else
	rhandle.source	   = source;
#endif
	rhandle.tag	   = tag;
	rhandle.contextid  = comm->recv_context;
	rhandle.comm	   = comm;
	rhandle.datatype   = datatype;
	datatype->ref_count ++;
	rhandle.bufadd	   = buf;
	rhandle.count	   = count;
	MPID_Clr_completed( comm->ADIctx, request );
	rhandle.persistent = 0;
#ifdef MPID_HAS_HETERO
	rhandle.msgrep     = MPIR_MSGREP_UNKNOWN;
#endif
	MPID_Alloc_recv_handle(rhandle.comm->ADIctx, &(rhandle.dev_rhandle));

	/* The next "routine" is a macro that sets mpi_errno */
	MPIR_RECV_SETUP_BUFFER( &request, rhandle );
	if (mpi_errno) 
	    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_RECV" );

	mpi_errno = MPID_Blocking_recv( rhandle.comm->ADIctx, &rhandle );

	status->count	       = rhandle.totallen;
	status->MPI_SOURCE     = rhandle.source;
	status->MPI_TAG        = rhandle.tag;

#ifdef MPID_RETURN_PACKED
	/* Count may change if size-changing conversion */
	if (!mpi_errno && rhandle.bufpos) 
	    mpi_errno = MPIR_UnPackMessage( buf, count, datatype, 
				        source, request, &status->count );
#endif
	rhandle.datatype->ref_count--;
    }
    else {
	/* See MPI standard section 3.11 */
	status->count	       = 0;
	status->MPI_SOURCE     = MPI_PROC_NULL;
	status->MPI_TAG        = MPI_ANY_TAG;
	}
    return MPI_SUCCESS;
}

