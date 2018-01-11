/*
 *  $Id: errget.c,v 1.1.1.1 1997/09/17 20:41:54 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Errhandler_get - Gets the error handler for a communicator

Input Parameter:
. comm - communicator to get the error handler from (handle) 

Output Parameter:
. errhandler - MPI error handler currently associated with communicator
(handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Errhandler_get( comm, errhandler )
MPI_Comm comm;
MPI_Errhandler *errhandler;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_ERRHANDLER_GET";

    TR_PUSH(myname);
    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    *errhandler = comm_ptr->error_handler;
    /* A get creates a reference to an error handler; the user must 
       explicitly free this reference */
    MPIR_Errhandler_mark( *errhandler, 1 );
    
    TR_POP;
    return MPI_SUCCESS;
}
