/*
 *  $Id: test.c,v 1.11 1994/07/13 04:04:35 lusk Exp $
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
    int              index;

    /* We let Testany detect errors */
    return MPI_Testany( 1, request, &index, flag, status );
}
