/*
 *  $Id: iprobe.c,v 1.4 1998/04/28 21:46:52 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@
    MPI_Iprobe - Nonblocking test for a message

Input Parameters:
+ source - source rank, or  'MPI_ANY_SOURCE' (integer) 
. tag - tag value or  'MPI_ANY_TAG' (integer) 
- comm - communicator (handle) 

Output Parameter:
+ flag - (logical) 
- status - status object (Status) 

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
    int mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_IPROBE";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);
    if (MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm_ptr,source)) {
	return MPIR_ERROR( comm_ptr, mpi_errno, myname );
    }
    if (source == MPI_PROC_NULL) {
	status->MPI_SOURCE = MPI_PROC_NULL;
	status->MPI_TAG	   = MPI_ANY_TAG;
	MPID_ZERO_STATUS_COUNT(status);
	return MPI_SUCCESS;
	}
    MPID_Iprobe( comm_ptr, tag, comm_ptr->recv_context, source, flag, 
		 &mpi_errno, status );
    TR_POP;
    MPIR_RETURN( comm_ptr, mpi_errno, myname );
}
