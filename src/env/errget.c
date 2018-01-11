/*
 *  $Id: errget.c,v 1.3 1994/07/13 15:50:05 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: errget.c,v 1.3 1994/07/13 15:50:05 lusk Exp $";
#endif

#include "mpiimpl.h"

/*@
  MPI_Errhandler_get - Gets the error handler for a communicator

Input Parameter:
. comm - communicator to get the error handler from (handle) 
Output Parameter:
. errhandler - MPI error handler currently associated with communicator
(handle) 

@*/
int MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler *errhandler;
{
int errno;
if (MPIR_TEST_COMM(comm,comm)) {
    return MPIR_ERROR( comm, errno, "Error in MPI_ERRHANDLER_GET" );
    }
else
    *errhandler = comm->error_handler;
return MPI_SUCCESS;
}
