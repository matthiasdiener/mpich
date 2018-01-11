/*
 *  $Id: comm_testic.c,v 1.6 1997/01/07 01:47:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_test_inter - Tests to see if a comm is an inter-communicator

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. flag - (logical) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Comm_test_inter ( comm, flag )
MPI_Comm  comm;
int      *flag;
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_COMM_TEST_INTER";

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    if (MPIR_TEST_ARG(flag) )
	return MPIR_ERROR( comm_ptr, mpi_errno, myname );
  
    *flag = (comm_ptr->comm_type == MPIR_INTER);

    return (mpi_errno);
}
