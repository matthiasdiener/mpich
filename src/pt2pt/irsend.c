/*
 *  $Id: irsend.c,v 1.5 1994/07/13 04:03:05 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: irsend.c,v 1.5 1994/07/13 04:03:05 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Irsend - Starts a nonblocking ready send

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
int MPI_Irsend( buf, count, datatype, dest, tag, comm, request )
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
        /* We'll let MPI_Rsend_init find the errors */
        if (errno = 
        MPI_Rsend_init( buf, count, datatype, dest, tag, comm, request ))
	    return errno;
	(*request)->shandle.persistent = 0;
	return MPI_Start( request );
    }
    else 
	*request = 0;
    return MPI_SUCCESS;
}
