/*
 *  $Id: probe.c,v 1.19 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Probe - Blocking test for a message

Input Parameters:
. source - source rank, or 'MPI_ANY_SOURCE' (integer) 
. tag - tag value or 'MPI_ANY_TAG' (integer) 
. comm - communicator (handle) 

Output Parameter:
. status - status object (Status) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TAG
.N MPI_ERR_RANK
@*/
int MPI_Probe( source, tag, comm, status )
int         source;
int         tag;
MPI_Comm    comm;
MPI_Status  *status;
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_PROBE";
    
    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    if (MPIR_TEST_RECV_TAG(comm,tag) || MPIR_TEST_RECV_RANK(comm_ptr,source))
	return MPIR_ERROR( comm_ptr, mpi_errno, myname );

    if (source == MPI_PROC_NULL) {
	status->MPI_SOURCE = MPI_PROC_NULL;
	status->MPI_TAG	   = MPI_ANY_TAG;
	status->count	   = 0;
	return MPI_SUCCESS;
	}
#ifdef MPI_ADI2
    MPID_Probe( comm_ptr, tag, comm_ptr->recv_context, source, &mpi_errno, 
		status );
    MPIR_RETURN(comm_ptr,mpi_errno,myname);
#else
#ifdef MPID_NEEDS_WORLD_SRC_INDICES
    MPID_Probe( comm->ADIctx, tag, 
	        (source >= 0) ? (comm->lrank_to_grank[source])  : source,
	        comm->recv_context, status );
#else
    MPID_Probe( comm->ADIctx, tag, 
	        source, 
	        comm->recv_context, status );
#endif
    return MPI_SUCCESS;
#endif
}
