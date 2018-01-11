/*
 *  $Id: testcancel.c,v 1.6 1994/07/13 15:51:10 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: testcancel.c,v 1.6 1994/07/13 15:51:10 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
  MPI_Test_cancelled - Tests to see if a request was canceled

Input Parameter:
. status - status object (Status) 

Output Parameter:
. flag - (logical) 
@*/
int MPI_Test_cancelled( status, flag )
MPI_Status *status;
int        *flag;
{
*flag = (status->MPI_TAG == MPIR_MSG_CANCELLED);

return MPI_SUCCESS;
}
