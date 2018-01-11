/*
 *  $Id: iprobe.c,v 1.10 1995/12/21 21:12:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: ";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

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
    return MPI_SUCCESS;
}
