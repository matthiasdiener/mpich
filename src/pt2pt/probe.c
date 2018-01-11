/*
 *  $Id: probe.c,v 1.17 1996/04/11 20:21:10 gropp Exp $
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
    int mpi_errno;
    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source))
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_PROBE" );

    if (source == MPI_PROC_NULL) {
	status->MPI_SOURCE = MPI_PROC_NULL;
	status->MPI_TAG	   = MPI_ANY_TAG;
	status->count	   = 0;
	return MPI_SUCCESS;
	}
#ifdef MPI_ADI2
    MPID_Probe( comm, tag, comm->recv_context, source, &mpi_errno, status );
    MPIR_RETURN(comm,mpi_errno,"Error in MPI_PROBE");
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
