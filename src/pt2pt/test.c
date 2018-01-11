/*
 *  $Id: test.c,v 1.13 1995/12/21 21:35:38 gropp Exp $
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

.N fortran
@*/
int MPI_Test ( request, flag, status )
MPI_Request  *request;
int          *flag;
MPI_Status   *status;
{
    /* We let Testall detect errors */
    return MPI_Testall( 1, request, flag, status );
}
