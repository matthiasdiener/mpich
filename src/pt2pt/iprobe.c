/*
 *  $Id: iprobe.c,v 1.6 1994/09/21 15:26:59 gropp Exp $
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
. source - source rank, or  MPI_ANY_SOURCE (integer) 
. tag - tag value or  MPI_ANY_TAG (integer) 
. comm - communicator (handle) 

Output Parameter:
. flag - (logical) 
. status - status object (Status) 

@*/
int MPI_Iprobe( source, tag, comm, flag, status )
int         source;
int         tag;
int         *flag;
MPI_Comm    comm;
MPI_Status  *status;
{
    int errno;
    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source))
	return MPIR_ERROR( comm, errno, "Error in MPI_PROBE" );

    MPID_Iprobe( comm->ADIctx, 
		 tag, source, comm->recv_context, flag, status );
    return MPI_SUCCESS;
}
