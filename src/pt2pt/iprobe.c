/*
 *  $Id: iprobe.c,v 1.11 1996/04/11 20:19:17 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Iprobe - Nonblocking test for a message

Input Parameters:
. source - source rank, or  'MPI_ANY_SOURCE' (integer) 
. tag - tag value or  'MPI_ANY_TAG' (integer) 
. comm - communicator (handle) 

Output Parameter:
. flag - (logical) 
. status - status object (Status) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TAG
.N MPI_ERR_RANK

@*/
int MPI_Iprobe( source, tag, comm, flag, status )
int         source;
int         tag;
int         *flag;
MPI_Comm    comm;
MPI_Status  *status;
{
    int mpi_errno;
    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source))
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_IPROBE" );

    if (source == MPI_PROC_NULL) {
	status->MPI_SOURCE = MPI_PROC_NULL;
	status->MPI_TAG	   = MPI_ANY_TAG;
	status->count	   = 0;
	return MPI_SUCCESS;
	}
#ifdef MPI_ADI2
    MPID_Iprobe( comm, tag, comm->recv_context, source, flag, &mpi_errno, 
		 status );
#else

#ifdef MPID_NEEDS_WORLD_SRC_INDICES
    MPID_Iprobe( comm->ADIctx, 
		 tag, 
		 (source >= 0) ? (comm->lrank_to_grank[source]) : source,
		 comm->recv_context, flag, status );
#else
    MPID_Iprobe( comm->ADIctx, 
		 tag, 
		 source, 
		 comm->recv_context, flag, status );
#endif
#endif
    return MPI_SUCCESS;
}
