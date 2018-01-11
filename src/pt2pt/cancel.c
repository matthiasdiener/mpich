/*
 *  $Id: cancel.c,v 1.15 1996/06/13 14:33:20 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Cancel - Cancels a communication request

Input Parameter:
. request - communication request (handle) 

Note:
Cancel has only been implemented for receive requests; it is a no-op for
send requests.  The primary expected use of MPI_Cancel is in multi-buffering
schemes, where speculative MPI_Irecvs are made.  When the computation 
completes, some of these receive requests may remain; using MPI_Cancel allows
the user to cancel these unsatisfied requests.  

Cancelling a send operation is much more difficult, in large part because the 
send will usually be at least partially complete (the information on the tag,
size, and source are usually sent immediately to the destination).  MPICH
will support this once other enhancements are completed; however, users are
advised that cancelling a send, while a local operation, is likely to 
be expensive (usually generating one or more internal messages).

.N fortran
@*/
int MPI_Cancel( request )
MPI_Request *request;
{
#ifdef MPI_ADI2
    int mpi_errno;
    switch ((*request)->handle_type) {
    case MPIR_SEND:
	MPID_SendCancel( *request, &mpi_errno );
	break;
    case MPIR_RECV:
	MPID_RecvCancel( *request, &mpi_errno );
	break;
    case MPIR_PERSISTENT_SEND:
	MPID_SendCancel( *request, &mpi_errno );
	break;
    case MPIR_PERSISTENT_RECV:
	MPID_RecvCancel( *request, &mpi_errno );
	break;
    /* For user request, cast and call user cancel function */
    }

    return MPI_SUCCESS;
#else
    /* Note that cancel doesn't have to actually DO anything... */
    /* Needs an ADI hook to insure that there are no race conditions in
       the access of the device or queue structures */
    MPID_CANCEL( (*request)->chandle.comm->ADIctx, &(*request)->chandle );
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INTERN, 
	       "MPI_Cancel not yet implemented");
   
    /* 
       Note that the standard requires MPI_WAIT etc to be called to actually
       complete the cancel; in those case, the wait operation should
       handle decrementingthe datatype->ref_count.
     */
    return MPI_SUCCESS;
#endif
}

