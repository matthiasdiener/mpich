/*
 *  $Id: recv.c,v 1.40 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
/*@
    MPI_Recv - Basic receive

Output Parameters:
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
number can be determined with 'MPI_Get_count'.  

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TYPE
.N MPI_ERR_COUNT
.N MPI_ERR_TAG
.N MPI_ERR_RANK

@*/
int MPI_Recv( buf, count, datatype, source, tag, comm, status )
void             *buf;
int              count, source, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    static char myname[] = "MPI_RECV";
#ifndef MPI_ADI2
    MPI_Request request;
    MPIR_RHANDLE rhandle;
#endif
    int         mpi_errno = MPI_SUCCESS;

    /* 
       Because this is a very common routine, we show how it can be
       optimized to be run "inline"; In addition, this lets us exploit
       features in the ADI to simplify the execution of blocking receive
       calls.
     */
    if (source != MPI_PROC_NULL)
    {
	comm_ptr = MPIR_GET_COMM_PTR(comm);
	MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

	dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
	MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

        if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_RECV_TAG(comm,tag) ||
	    MPIR_TEST_RECV_RANK(comm_ptr,source)) 
	    return MPIR_ERROR(comm_ptr, mpi_errno, myname );

#ifdef MPI_ADI2
	MPID_RecvDatatype( comm_ptr, buf, count, dtype_ptr, source, tag, 
			   comm_ptr->recv_context, status, &mpi_errno );
	MPIR_RETURN(comm_ptr, mpi_errno, myname );
#else
        request = (MPI_Request)&rhandle;
	request->type	   = MPIR_RECV;
#ifdef MPID_NEEDS_WORLD_SRC_INDICES
        rhandle.source = (source >= 0) ? (comm_ptr->lrank_to_grank[source])
                                         : source;
#else
	rhandle.source	   = source;
#endif
	rhandle.tag	   = tag;
	rhandle.contextid  = comm_ptr->recv_context;
	rhandle.errval     = MPI_SUCCESS;
	rhandle.comm	   = comm;
/*	MPIR_GET_REAL_DATATYPE(datatype) */
	rhandle.datatype   = datatype;
/*	datatype->ref_ count ++; */
	rhandle.bufadd	   = buf;
	rhandle.count	   = count;
	MPID_Clr_completed( comm_ptr->ADIctx, request );
	rhandle.persistent = 0;
#ifdef MPID_HAS_HETERO
	rhandle.msgrep     = MPIR_MSGREP_UNKNOWN;
#endif
	MPID_Alloc_recv_handle(rhandle.comm_ptr->ADIctx, &(rhandle.dev_rhandle));

	/* The next "routine" is a macro that sets mpi_errno */
	MPIR_RECV_SETUP_BUFFER( &request, rhandle );
	if (mpi_errno) 
	    return MPIR_ERROR( comm, mpi_errno, myname );

	MPIR_CALL(MPID_Blocking_recv( rhandle.comm_ptr->ADIctx, &rhandle ),
		   comm, myname );

	status->count	       = rhandle.totallen;
	status->MPI_SOURCE     = rhandle.source;
	status->MPI_TAG        = rhandle.tag;

#ifdef MPID_RETURN_PACKED
	/* Count may change if size-changing conversion */
	if (!mpi_errno && rhandle.bufpos) {
	    MPIR_CALL(MPIR_UnPackMessage( buf, count, datatype, 
				        source, request, &status->count ),
	    comm, myname );
	    }
#endif
/* 	rhandle.datatype->ref_ count--; */
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

