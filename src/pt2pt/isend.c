/*
 *  $Id: isend.c,v 1.16 1995/05/09 18:10:17 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: isend.c,v 1.16 1995/05/09 18:10:17 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Isend - Begins a nonblocking send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 
@*/
int MPI_Isend( buf, count, datatype, dest, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              dest;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    int err;
    
    /* We'll let MPI_Send_init routine detect the errors */
    err = MPI_Send_init( buf, count, datatype, dest, tag, comm, request );
    if (err)
	return err;
    
    (*request)->shandle.persistent = 0;
    
    if (dest != MPI_PROC_NULL) {
	return MPI_Start( request );
	}

    /*
       This must create a completed request so that we can wait on it
     */
    MPID_Set_completed( comm->ADIctx, *request );
    (*request)->shandle.active     = 1;
    return MPI_SUCCESS;
}
