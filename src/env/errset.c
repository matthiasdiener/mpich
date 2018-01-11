/*
 *  $Id: errset.c,v 1.4 1994/09/13 21:48:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: errset.c,v 1.4 1994/09/13 21:48:08 gropp Exp $";
#endif

#include "mpiimpl.h"

/*@
  MPI_Errhandler_set - Sets the error handler for a communicator

Input Parameters:
. comm - communicator to set the error handler for (handle) 
. errhandler - new MPI error handler for communicator (handle) 

@*/
int MPI_Errhandler_set( comm, errhandler )
MPI_Comm       comm;
MPI_Errhandler errhandler;
{
int errno;
if (MPIR_TEST_COMM(comm,comm)) {
    return MPIR_ERROR( comm, errno, "Error in MPI_ERRHANDLER_SET" );
    }
else {
    comm->error_handler = errhandler;
    errhandler->ref_count ++;
    }
return MPI_SUCCESS;
}
