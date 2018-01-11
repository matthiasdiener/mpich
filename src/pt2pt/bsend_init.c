/*
 *  $Id: bsend_init.c,v 1.3 1998/04/28 21:46:39 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "reqalloc.h"

/*@
    MPI_Bsend_init - Builds a handle for a buffered send

Input Parameters:
+ buf - initial address of send buffer (choice) 
. count - number of elements sent (integer) 
. datatype - type of each element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_RANK
.N MPI_ERR_TAG
@*/
int MPI_Bsend_init( buf, count, datatype, dest, tag, comm, request )
void          *buf;
int           count;
MPI_Datatype  datatype;
int           dest;
int           tag;
MPI_Comm      comm;
MPI_Request   *request;
{
    int         mpi_errno;
    int         psize;
    void        *bufp;
    struct MPIR_DATATYPE *dtype_ptr;
    struct MPIR_COMMUNICATOR *comm_ptr;
    MPIR_PSHANDLE *shandle;
    MPIR_ERROR_DECL;
    static char myname[] = "MPI_BSEND_INIT";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_SEND_RANK(comm_ptr,dest) || MPIR_TEST_SEND_TAG(comm,tag))
	return MPIR_ERROR( comm_ptr, mpi_errno, myname );

    MPIR_ALLOCFN(shandle,MPID_PSendAlloc,
	       comm_ptr,MPI_ERR_EXHAUSTED,myname);
    *request = (MPI_Request)shandle;
    MPID_Request_init( &(shandle->shandle), MPIR_PERSISTENT_SEND );
    /* Save the information about the operation, being careful with
       ref-counted items */
    MPIR_REF_INCR(dtype_ptr);
    shandle->perm_datatype = dtype_ptr;
    shandle->perm_tag	   = tag;
    shandle->perm_dest	   = dest;
    shandle->perm_count	   = count;
    shandle->perm_buf	   = buf;
    MPIR_REF_INCR(comm_ptr);
    shandle->perm_comm	   = comm_ptr;
    shandle->active	   = 0;
    shandle->send          = MPIR_IbsendDatatype;

    if (dest != MPI_PROC_NULL) {
	MPIR_ERROR_PUSH(comm_ptr);
	/* Allocate space if needed */
	MPIR_CALL_POP(MPI_Pack_size( count, datatype, comm, &psize ),
		      comm_ptr,myname);
	MPIR_CALL_POP(MPIR_BsendAlloc( psize, *request, &bufp ),comm_ptr,myname);
	/* Information stored in the bsend part by BsendAlloc */
/* 	shandle->shandle.start = bufp;
	shandle->shandle.bytes_as_contig = psize; */
	MPIR_ERROR_POP(comm_ptr);
    }
    else 
	/* Rest of dest of MPI_PROC_NULL handled in start */
	shandle->shandle.start = 0;

    TR_POP;
    return MPI_SUCCESS;
}
