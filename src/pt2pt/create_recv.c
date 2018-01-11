/*
 *  $Id: create_recv.c,v 1.19 1996/06/07 15:07:30 gropp Exp $
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
    MPI_Recv_init - Builds a handle for a receive

Input Parameters:
. buf - initial address of receive buffer (choice) 
. count - number of elements received (integer) 
. datatype - type of each element (handle) 
. source - rank of source or 'MPI_ANY_SOURCE' (integer) 
. tag - message tag or 'MPI_ANY_TAG' (integer) 
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
#ifdef MPI_ADI2
    MPIR_PRHANDLE *rhandle;
#else
    MPI_Request handleptr;
#endif

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_RECV_INIT" );

#ifdef MPI_ADI2
    MPIR_ALLOC(*request,(MPI_Request)MPID_PRecvAlloc(),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_RECV_INIT" );
    rhandle = &(*request)->persistent_rhandle;
    MPID_Request_init( &(rhandle->rhandle), MPIR_PERSISTENT_RECV );
    /* Save the information about the operation, being careful with
       ref-counted items */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    rhandle->perm_datatype = datatype;
    rhandle->perm_tag	   = tag;
    rhandle->perm_source   = source;
    rhandle->perm_count	   = count;
    rhandle->perm_buf	   = buf;
    comm->ref_count++;
    rhandle->perm_comm	   = comm;
    rhandle->active	   = 0;
    /* dest of MPI_PROC_NULL handled in start */
#else
    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    MPIR_ALLOC(handleptr,(MPI_Request) MPIR_SBalloc( MPIR_rhandles ),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_RECV_INIT");
    *request			   = handleptr;
    MPIR_SET_COOKIE(&handleptr->rhandle,MPIR_REQUEST_COOKIE)
    handleptr->type		   = MPIR_RECV;
#ifdef MPID_NEEDS_WORLD_SRC_INDICES
    handleptr->rhandle.source	   = 
	(source >= 0) ? (comm->lrank_to_grank[source]) : source;
#else
    handleptr->rhandle.source	   = source;
#endif
    handleptr->rhandle.tag	   = tag;
    handleptr->rhandle.errval	   = MPI_SUCCESS;
    handleptr->rhandle.contextid   = comm->recv_context;
    handleptr->rhandle.comm	   = comm;
    handleptr->rhandle.datatype	   = datatype;
    handleptr->rhandle.bufadd	   = buf;
    handleptr->rhandle.count	   = count;
    handleptr->rhandle.persistent  = 1;
    handleptr->rhandle.active	   = 0;
    handleptr->rhandle.perm_source = source;
    handleptr->rhandle.perm_tag	   = tag;
    

    if (source == MPI_PROC_NULL) {
	MPID_Set_completed(  comm->ADIctx, handleptr );
	handleptr->rhandle.bufpos = 0;
	}
    else {
	MPID_Clr_completed(  comm->ADIctx, handleptr );
	}
    MPID_Alloc_recv_handle(handleptr->rhandle.comm->ADIctx,
			   &((handleptr)->rhandle.dev_rhandle));
#endif
    return MPI_SUCCESS;
}
