/*
 *  $Id: wait.c,v 1.12 1995/12/21 21:41:40 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: wait.c,v 1.12 1995/12/21 21:41:40 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Wait  - Waits for an MPI send or receive to complete

Input Parameter:
. request - request (handle) 

Output Parameter:
. status - status object (Status) 

.N fortran
@*/
int MPI_Wait ( request, status )
MPI_Request  *request;
MPI_Status   *status;
{
    int mpi_errno;
    /* We'll let MPI_Waitall catch the errors */
    mpi_errno = MPI_Waitall( 1, request, status );
    return mpi_errno;
}
