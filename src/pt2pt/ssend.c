/*
 *  $Id: ssend.c,v 1.3 1998/04/28 21:47:12 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@
    MPI_Ssend - Basic synchronous send

Input Parameters:
+ buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (nonnegative integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

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
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_SSEND";
    
    if (dest != MPI_PROC_NULL)
    {
	comm_ptr = MPIR_GET_COMM_PTR(comm);
	MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,"MPI_SSEND");

	MPIR_ERROR_PUSH(comm_ptr);
	MPIR_CALL_POP(MPI_Issend( buf, count, datatype, dest, tag, comm, 
			        &handle ),comm_ptr,myname);

	MPIR_CALL_POP(MPI_Wait( &handle, &status ),comm_ptr,myname);
    }
    return mpi_errno;
}
