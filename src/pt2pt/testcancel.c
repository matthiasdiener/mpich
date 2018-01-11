/*
 *  $Id: testcancel.c,v 1.2 1998/01/29 14:28:36 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Test_cancelled - Tests to see if a request was cancelled

Input Parameter:
. status - status object (Status) 

Output Parameter:
. flag - (logical) 

.N fortran
@*/
int MPI_Test_cancelled( status, flag )
MPI_Status *status;
int        *flag;
{
    *flag = (status->MPI_TAG == MPIR_MSG_CANCELLED);

    return MPI_SUCCESS;
}
