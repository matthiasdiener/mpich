/*
 *  $Id: ssend.c,v 1.7 1996/04/11 20:22:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


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

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
@*/
int MPI_Ssend( buf, count, datatype, dest, tag, comm )
void             *buf;
int              count, dest, tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
{
    int         mpi_errno = MPI_SUCCESS;
    MPI_Request handle;
    MPI_Status  status;
    MPIR_ERROR_DECL;
    static char myname[] = "Error in MPI_Ssend";

    if (dest != MPI_PROC_NULL)
    {
	MPIR_ERROR_PUSH(comm);
	MPIR_CALL_POP(MPI_Issend( buf, count, datatype, dest, tag, comm, 
			        &handle ),comm,myname);
	MPIR_CALL_POP(MPI_Wait( &handle, &status ),comm,myname);
    }
    return mpi_errno;
}
