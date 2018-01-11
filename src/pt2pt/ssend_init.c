/*
 *  $Id: ssend_init.c,v 1.19 1996/06/07 15:07:30 gropp Exp $
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
    MPI_Ssend_init - Builds a handle for a synchronous send

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
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
@*/
int MPI_Ssend_init( buf, count, datatype, dest, tag, comm, request )
void          *buf;
int           count;
MPI_Datatype  datatype;
int           dest;
int           tag;
MPI_Comm      comm;
MPI_Request   *request;
{
    int         mpi_errno;
#ifdef MPI_ADI2
    MPIR_PSHANDLE *shandle;
#else
    MPI_Request handleptr;
#endif

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm,dest)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_SSEND_INIT" );

#ifdef MPI_ADI2
    /* This is IDENTICAL to the create_send code except for the send
       function */
    MPIR_ALLOC(*request,(MPI_Request)MPID_PSendAlloc(),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_SSEND_INIT" );
    shandle = &(*request)->persistent_shandle;
    MPID_Request_init( &(shandle->shandle), MPIR_PERSISTENT_SEND );
    /* Save the information about the operation, being careful with
       ref-counted items */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    shandle->perm_datatype = datatype;
    shandle->perm_tag	   = tag;
    shandle->perm_dest	   = dest;
    shandle->perm_count	   = count;
    shandle->perm_buf	   = buf;
    comm->ref_count++;
    shandle->perm_comm	   = comm;
    shandle->active	   = 0;
    shandle->send          = MPID_IssendDatatype;
    /* dest of MPI_PROC_NULL handled in start */
#else
    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    MPIR_ALLOC(handleptr,(MPI_Request) MPIR_SBalloc( MPIR_shandles ),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_SSEND_INIT");
    *request = handleptr;
    MPIR_SET_COOKIE(&handleptr->shandle,MPIR_REQUEST_COOKIE)
    handleptr->type                 = MPIR_SEND;
    if (dest == MPI_PROC_NULL) {
	handleptr->shandle.dest	  = dest;
	MPID_Set_completed( comm->ADIctx, handleptr );
	handleptr->shandle.bufpos = 0;
	}
    else {
	handleptr->shandle.dest     = comm->group->lrank_to_grank[dest];
	MPID_Clr_completed( comm->ADIctx, handleptr );
	}
    handleptr->shandle.tag          = tag;
    handleptr->shandle.contextid    = comm->send_context;
    handleptr->shandle.comm         = comm;
    handleptr->shandle.lrank        = comm->local_group->local_rank;
    handleptr->shandle.mode         = MPIR_MODE_SYNCHRONOUS;
    handleptr->shandle.datatype     = datatype;
    handleptr->shandle.bufadd       = buf;
    handleptr->shandle.count        = count;
    handleptr->rhandle.persistent  = 1;
    handleptr->rhandle.active      = 0;
#ifdef MPID_HAS_HETERO
    handleptr->shandle.msgrep	    = MPIR_MSGREP_SENDER;
#endif
    MPID_Alloc_send_handle( comm->ADIctx, 
			   &((handleptr)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm->ADIctx, 
				 &((handleptr)->shandle.dev_shandle), 1 );
#endif
    return MPI_SUCCESS;
}
