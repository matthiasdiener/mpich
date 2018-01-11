/*
 *  $Id: create_send.c,v 1.23 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "reqalloc.h"
#else
#include "mpisys.h"
#endif

/*@
    MPI_Send_init - Builds a handle for a standard send

Input Parameters:
. buf - initial address of send buffer (choice) 
. count - number of elements sent (integer) 
. datatype - type of each element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
. comm - communicator (handle) 
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
int MPI_Send_init( buf, count, datatype, dest, tag, comm, request )
void          *buf;
int           count;
MPI_Datatype  datatype;
int           dest;
int           tag;
MPI_Comm      comm;
MPI_Request   *request;
{
    int         mpi_errno;
    struct MPIR_DATATYPE *dtype_ptr;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_SEND_INIT";
#ifdef MPI_ADI2
    MPIR_PSHANDLE *shandle;
#else
    MPI_Request handleptr;
#endif

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (MPIR_TEST_COUNT(comm,count) || MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm_ptr,dest)) 
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );

#ifdef MPI_ADI2
    MPIR_ALLOC(*request,(MPI_Request)MPID_PSendAlloc(),
	       comm_ptr,MPI_ERR_EXHAUSTED,myname );
    shandle = &(*request)->persistent_shandle;
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
    shandle->send          = MPID_IsendDatatype;
    /* dest of MPI_PROC_NULL handled in start */
#else
    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    MPIR_REF_INCR(dtype_ptr);
    MPIR_ALLOC(handleptr,(MPI_Request) MPIR_SBalloc( MPIR_shandles ),comm_ptr,
	       MPI_ERR_EXHAUSTED,myname);
    *request = handleptr;
    MPIR_SET_COOKIE(&handleptr->shandle,MPIR_REQUEST_COOKIE)
    handleptr->type                 = MPIR_SEND;
    if (dest == MPI_PROC_NULL) {
	handleptr->shandle.dest	  = dest;
	MPID_Set_completed(  comm_ptr->ADIctx, handleptr );
	handleptr->shandle.active = 1;
	handleptr->shandle.bufpos = 0;
	}
    else {
	handleptr->shandle.dest     = comm_ptr->lrank_to_grank[dest];
	MPID_Clr_completed(  comm_ptr->ADIctx, handleptr );
	handleptr->shandle.active       = 0;
	}
    handleptr->shandle.tag          = tag;
    handleptr->shandle.contextid    = comm_ptr->send_context;
    handleptr->shandle.comm         = comm;
    handleptr->shandle.lrank        = comm_ptr->local_rank;
    handleptr->shandle.mode         = MPIR_MODE_STANDARD;
    handleptr->shandle.datatype     = datatype;
    handleptr->shandle.bufadd       = buf;
    handleptr->shandle.count        = count;
    handleptr->shandle.persistent   = 1;
#ifdef MPID_HAS_HETERO
    handleptr->shandle.msgrep	    = MPIR_MSGREP_SENDER;
#endif
    MPID_Alloc_send_handle(comm_ptr->ADIctx, &((handleptr)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm_ptr->ADIctx, 
				 &((handleptr)->shandle.dev_shandle), 1 );
#endif
    TR_POP;
    return MPI_SUCCESS;
}
