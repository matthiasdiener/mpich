/*
 *  $Id: probe.c,v 1.13 1994/12/21 14:30:39 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: probe.c,v 1.13 1994/12/21 14:30:39 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Probe - Blocking test for a message

Input Parameters:
. source - source rank, or MPI_ANY_SOURCE (integer) 
. tag - tag value or MPI_ANY_TAG (integer) 
. comm - communicator (handle) 

Output Parameter:
. status - status object (Status) 
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

    MPID_Probe( comm->ADIctx, tag, source, comm->recv_context, status );
    return MPI_SUCCESS;
}
