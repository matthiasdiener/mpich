/*
 *  $Id: probe.c,v 1.4 1998/04/28 21:47:02 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@
    MPI_Probe - Blocking test for a message

Input Parameters:
+ source - source rank, or 'MPI_ANY_SOURCE' (integer) 
. tag - tag value or 'MPI_ANY_TAG' (integer) 
- comm - communicator (handle) 

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
	MPID_ZERO_STATUS_COUNT(status);
	return MPI_SUCCESS;
	}
    MPID_Probe( comm_ptr, tag, comm_ptr->recv_context, source, &mpi_errno, 
		status );
    MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
