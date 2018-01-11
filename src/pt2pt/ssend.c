/*
 *  $Id: ssend.c,v 1.4 1994/07/13 04:04:14 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: ssend.c,v 1.4 1994/07/13 04:04:14 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Ssend - Basic synchronous send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

@*/
int MPI_Ssend( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int         errno = MPI_SUCCESS;
    MPI_Request handle;
    MPI_Status  status;

    if (dest != MPI_PROC_NULL)
    {
        
	errno = MPI_Issend( buf, count, datatype, dest, tag, comm, &handle );
	if (!errno)
	    errno = MPI_Wait( &handle, &status );
    }
    return errno;
}
