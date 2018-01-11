/*
 *  $Id: test.c,v 1.12 1995/05/09 18:59:17 gropp Exp $
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
@*/
int MPI_Test ( request, flag, status )
MPI_Request  *request;
int          *flag;
MPI_Status   *status;
{
    /* We let Testall detect errors */
    return MPI_Testall( 1, request, flag, status );
}
