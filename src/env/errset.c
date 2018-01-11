/*
 *  $Id: errset.c,v 1.2 1998/04/28 21:08:54 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Errhandler_set - Sets the error handler for a communicator

Input Parameters:
+ comm - communicator to set the error handler for (handle) 
- errhandler - new MPI error handler for communicator (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Errhandler_set( comm, errhandler )
MPI_Comm       comm;
MPI_Errhandler errhandler;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_Errhandler *old;
    static char myname[] = "MPI_ERRHANDLER_SET";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    old = MPIR_GET_ERRHANDLER_PTR( errhandler );
    MPIR_TEST_MPI_ERRHANDLER(errhandler,old,comm_ptr,myname);
    
    MPIR_REF_INCR(old);

    if (comm_ptr->error_handler) 
	MPI_Errhandler_free( &comm_ptr->error_handler );
    comm_ptr->error_handler = errhandler;

    TR_POP;
    return MPI_SUCCESS;
}
