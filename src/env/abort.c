/*
 *  $Id: abort.c,v 1.7 1994/09/29 21:51:11 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
   MPI_Abort - Terminates MPI execution environment

Input Parameters:
. comm - communicator of tasks to abort 
. errorcode - error code to return to invoking environment 

Notes:
Terminates all MPI processes associated with the communicator comm; in
most systems (all to date), terminates ALL processes.

@*/
int MPI_Abort( comm, errorcode )
MPI_Comm         comm;
int              errorcode;
{
MPID_ABORT( comm->ADIctx, errorcode );

/* If for some reason we get here, force an abort */
abort( );

/* This keeps lint happy */
return MPI_ERR_UNKNOWN;
}
