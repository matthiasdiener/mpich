/*
 *  $Id: issend.c,v 1.6 1995/03/05 20:16:33 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: issend.c,v 1.6 1995/03/05 20:16:33 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Issend - Starts a nonblocking synchronous send

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
int MPI_Issend( buf, count, datatype, dest, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              dest;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    int err;
    if (dest != MPI_PROC_NULL)
    {
        /* We'll let MPI_Ssend_init find the errors */
        if (err = 
        MPI_Ssend_init( buf, count, datatype, dest, tag, comm, request ))
	    return err;
	(*request)->shandle.persistent = 0;
	return MPI_Start( request );
    }
    else {
	/*
	   This must create a completed request so that we can wait on it
	 */
	if (err = 
	    MPI_Send_init( buf, count, datatype, dest, tag, comm, request ))
	    return err;
	MPID_Set_completed( comm->ADIctx, *request );
	(*request)->shandle.persistent = 0;
	(*request)->shandle.active     = 1;
	}
    return MPI_SUCCESS;
}
