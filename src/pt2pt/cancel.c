/*
 *  $Id: cancel.c,v 1.19 1997/01/17 22:59:08 gropp Exp $
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

.N NULL

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_REQUEST
.N MPI_ERR_ARG
@*/
int MPI_Cancel( request )
MPI_Request *request;
{
    static char myname[] = "MPI_CANCEL";
#ifdef MPI_ADI2
    int mpi_errno = MPI_SUCCESS;

    TR_PUSH(myname);

    if (MPIR_TEST_ARG(request))
	return MPIR_ERROR(MPIR_COMM_WORLD,mpi_errno,myname );
    
    /* A null request requires no effort to cancel.  However, it
       is an error. */
    if (*request == MPI_REQUEST_NULL) 
	return MPIR_ERROR(MPIR_COMM_WORLD,MPI_ERR_REQUEST_NULL,myname);

    /* Check that the request is actually a request */
    if (MPIR_TEST_REQUEST(MPI_COMM_WORLD,*request))
	return MPIR_ERROR(MPIR_COMM_WORLD,mpi_errno,myname);
    
    switch ((*request)->handle_type) {
    case MPIR_SEND:
	MPID_SendCancel( *request, &mpi_errno );
	break;
    case MPIR_RECV:
	MPID_RecvCancel( *request, &mpi_errno );
	break;
    case MPIR_PERSISTENT_SEND:
	/* Only active persistent operations can be cancelled */
	if (!(*request)->persistent_shandle.active)
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );
	MPID_SendCancel( *request, &mpi_errno );
	break;
    case MPIR_PERSISTENT_RECV:
	/* Only active persistent operations can be cancelled */
	if (!(*request)->persistent_rhandle.active)
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );
	MPID_RecvCancel( *request, &mpi_errno );
	break;
    /* For user request, cast and call user cancel function */
    }

    TR_POP;
    /* Note that we should really use the communicator in the request,
       if available! */
    MPIR_RETURN( MPIR_COMM_WORLD, mpi_errno, myname );
#else
    /* Note that cancel doesn't have to actually DO anything... */
    /* Needs an ADI hook to insure that there are no race conditions in
       the access of the device or queue structures */
    MPID_CANCEL( (*request)->chandle.comm->ADIctx, &(*request)->chandle );
    MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_INTERN, 
	       "MPI_Cancel not yet implemented");
   
    /* 
       Note that the standard requires MPI_WAIT etc to be called to actually
       complete the cancel; in those case, the wait operation should
       handle decrementingthe datatype->ref_count.
     */
    return MPI_SUCCESS;
#endif
}

