/*
 *  $Id: abort.c,v 1.11 1997/01/07 01:46:11 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpiimpl.h"

/*@
   MPI_Abort - Terminates MPI execution environment

Input Parameters:
. comm - communicator of tasks to abort 
. errorcode - error code to return to invoking environment 

Notes:
Terminates all MPI processes associated with the communicator 'comm'; in
most systems (all to date), terminates `all` processes.

.N fortran
@*/
int MPI_Abort( comm, errorcode )
MPI_Comm         comm;
int              errorcode;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_ABORT";

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);
    
#ifdef MPI_ADI2
    MPID_Abort( comm_ptr, errorcode, "MPI Abort by user", (char *)0 );
#else
    MPID_ABORT( comm->ADIctx, errorcode );
#endif

/* If for some reason we get here, force an abort */
    abort( );

/* This keeps lint happy */
    return MPI_ERR_UNKNOWN;
}
