/*
 *  $Id: cancel.c,v 1.12 1995/12/21 21:11:48 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: cancel.c,v 1.12 1995/12/21 21:11:48 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Cancel - Cancels a communication request

Input Parameter:
. request - communication request (handle) 

Note:
Cancel has not been implemented yet.

.N fortran
@*/
int MPI_Cancel( request )
MPI_Request *request;
{
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
}

