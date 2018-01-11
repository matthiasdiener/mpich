/*
 *  $Id: irecv.c,v 1.15 1995/03/05 20:16:06 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: irecv.c,v 1.15 1995/03/05 20:16:06 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Irecv - Begins a nonblocking receive

Input Parameters:
. buf - initial address of receive buffer (choice) 
. count - number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 
@*/
int MPI_Irecv( buf, count, datatype, source, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              source;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    int err;

    if (source != MPI_PROC_NULL)
    {
        /* We'll let this routine catch the errors */
        if (err = 
	    MPI_Recv_init( buf, count, datatype, source, tag, comm, request ))
	    return err;
	(*request)->rhandle.persistent = 0;
	return MPI_Start( request );
    }
    else {
	if (err = 
	    MPI_Recv_init( buf, count, datatype, source, tag, comm, request ))
	    return err;
	MPID_Set_completed( comm->ADIctx, *request );
	(*request)->rhandle.persistent = 0;
	(*request)->rhandle.active     = 1;
	}
    return MPI_SUCCESS;
}
