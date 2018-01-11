/*
 *  $Id: bsend.c,v 1.7 1994/12/15 17:11:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: bsend.c,v 1.7 1994/12/15 17:11:07 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Bsend - Basic send with user-specified buffering

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 

Notes:
This send is provided as a convenience function; it allows the user to 
send messages without worring about where they are buffered (because the
user MUST have provided buffer space with MPI_Buffer_attach).
@*/
int MPI_Bsend( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    MPI_Request handle;
    MPI_Status  status;
    int         mpi_errno = MPI_SUCCESS;

    if (dest != MPI_PROC_NULL)
    {
        if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	    MPIR_TEST_DATATYPE(comm,datatype) || 
	    MPIR_TEST_SEND_RANK(comm,dest) || MPIR_TEST_SEND_TAG(comm,tag))
	    MPIR_ERROR( comm, mpi_errno, "Error in MPI_Bsend" );

	MPI_Ibsend( buf, count, datatype, dest, tag, comm, &handle );
	/* This Wait is WRONG, but there isn't a good place to put it.
	   One approach would be for other waits to test the buffered
	   send list if it is non-empty ... 
         */
	mpi_errno = MPI_Wait( &handle, &status );
    }
    return mpi_errno;
}
