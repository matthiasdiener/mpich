/*
 *  $Id: ibsend.c,v 1.6 1994/10/27 17:44:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: ibsend.c,v 1.6 1994/10/27 17:44:24 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Ibsend - Starts a nonblocking buffered send

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
int MPI_Ibsend( buf, count, datatype, dest, tag, comm, request )
void             *buf;
int              count;
MPI_Datatype     datatype;
int              dest;
int              tag;
MPI_Comm         comm;
MPI_Request      *request;
{
    int errno;
    if (dest != MPI_PROC_NULL)
    {
        /* We'll let MPI_Bsend_init find the errors */
        if (errno = 
        MPI_Bsend_init( buf, count, datatype, dest, tag, comm, request ))
	    return errno;
	(*request)->shandle.persistent = 0;
	return MPI_Start( request );
    }
    else 
	*request = 0;
    return MPI_SUCCESS;
}
