/*
 *  $Id: create_recv.c,v 1.3 1998/04/28 21:46:44 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Recv_init - Builds a handle for a receive

Input Parameters:
+ buf - initial address of receive buffer (choice) 
. count - number of elements received (integer) 
. datatype - type of each element (handle) 
. source - rank of source or 'MPI_ANY_SOURCE' (integer) 
. tag - message tag or 'MPI_ANY_TAG' (integer) 
- comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_RANK
.N MPI_ERR_TAG
.N MPI_ERR_COMM
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Start, MPI_Request_free
@*/
int MPI_Recv_init( buf, count, datatype, source, tag, comm, request )
void         *buf;
int          count;
MPI_Request  *request;
MPI_Datatype datatype;
int          source;
int          tag;
MPI_Comm     comm;
{
    int         mpi_errno;
    struct MPIR_DATATYPE *dtype_ptr;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_RECV_INIT";
    MPIR_PRHANDLE *rhandle;

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);
    
    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm_ptr,source)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

    MPIR_ALLOCFN(rhandle,MPID_PRecvAlloc,
	       comm_ptr,MPI_ERR_EXHAUSTED,myname );
    *request = (MPI_Request)rhandle;
    MPID_Request_init( &(rhandle->rhandle), MPIR_PERSISTENT_RECV );
    /* Save the information about the operation, being careful with
       ref-counted items */
    MPIR_REF_INCR(dtype_ptr);
    rhandle->perm_datatype = dtype_ptr;
    rhandle->perm_tag	   = tag;
    rhandle->perm_source   = source;
    rhandle->perm_count	   = count;
    rhandle->perm_buf	   = buf;
    MPIR_REF_INCR(comm_ptr);
    rhandle->perm_comm	   = comm_ptr;
    rhandle->active	   = 0;
    /* dest of MPI_PROC_NULL handled in start */

    TR_POP;
    return MPI_SUCCESS;
}
