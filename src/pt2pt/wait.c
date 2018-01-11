/*
 *  $Id: wait.c,v 1.10 1994/07/13 15:53:21 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: wait.c,v 1.10 1994/07/13 15:53:21 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Wait  - Waits for an MPI send or receive to complete

Input Parameter:
. request - request (handle) 

Output Parameter:
. status - status object (Status) 
@*/
int MPI_Wait ( request, status )
MPI_Request  *request;
MPI_Status   *status;
{
    int errno;
    /* We'll let MPI_Waitall catch the errors */
    errno = MPI_Waitall( 1, request, status );
    return errno;
}
