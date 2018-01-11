/*
 *  $Id: wait.c,v 1.13 1996/04/11 20:26:44 gropp Exp $
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

    MPIR_ERROR_PUSH(MPI_COMM_WORLD);
    /* We'll let MPI_Waitall catch the errors */
    mpi_errno = MPI_Waitall( 1, request, status );
    MPIR_ERROR_POP(MPI_COMM_WORLD);
    if (mpi_errno == MPI_ERR_IN_STATUS)
	mpi_errno = status->MPI_ERROR;

    MPIR_RETURN(MPI_COMM_WORLD,mpi_errno,"Error in MPI_WAIT");
}
