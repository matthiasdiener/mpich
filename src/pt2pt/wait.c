/*
 *  $Id: wait.c,v 1.2 1998/01/16 16:27:00 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@
    MPI_Wait  - Waits for an MPI send or receive to complete

Input Parameter:
. request - request (handle) 

Output Parameter:
. status - status object (Status) 

.N waitstatus

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
@*/
int MPI_Wait ( request, status )
MPI_Request  *request;
MPI_Status   *status;
{
    int mpi_errno;
    MPIR_ERROR_DECL;

    MPIR_ERROR_PUSH(MPIR_COMM_WORLD);
    /* We'll let MPI_Waitall catch the errors */
    mpi_errno = MPI_Waitall( 1, request, status );

    MPIR_ERROR_POP(MPIR_COMM_WORLD);
    if (mpi_errno == MPI_ERR_IN_STATUS)
	mpi_errno = status->MPI_ERROR;

    MPIR_RETURN(MPIR_COMM_WORLD,mpi_errno,"MPI_WAIT");
}
