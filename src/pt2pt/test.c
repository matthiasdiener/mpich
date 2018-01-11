/*
 *  $Id: test.c,v 1.16 1997/01/24 21:55:18 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
    MPI_Test  - Tests for the completion of a send or receive

Input Parameter:
. request - communication request (handle) 

Output Parameter:
. flag - true if operation completed (logical) 
. status - status object (Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
@*/
int MPI_Test ( request, flag, status )
MPI_Request  *request;
int          *flag;
MPI_Status   *status;
{
    int mpi_errno;
    MPIR_ERROR_DECL;

    MPIR_ERROR_PUSH(MPIR_COMM_WORLD);
    /* We let Testall detect errors */
    mpi_errno = MPI_Testall( 1, request, flag, status );
    MPIR_ERROR_POP(MPIR_COMM_WORLD);
    if (mpi_errno == MPI_ERR_IN_STATUS) 
	mpi_errno = status->MPI_ERROR;
    MPIR_RETURN(MPIR_COMM_WORLD, mpi_errno, "MPI_TEST" );
}
