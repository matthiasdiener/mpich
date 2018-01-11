/*
 *  $Id: bsend_init.c,v 1.25 1996/06/07 15:00:49 gropp Exp $
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
    MPI_Bsend_init - Builds a handle for a buffered send

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
#ifdef MPI_ADI2
    MPIR_PSHANDLE *shandle;
#else
    MPI_Request handleptr;
#endif
    MPIR_ERROR_DECL;
    static char myname[] = "Error in MPI_Bsend_init";

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || 
	MPIR_TEST_SEND_RANK(comm,dest) || MPIR_TEST_SEND_TAG(comm,tag))
	return MPIR_ERROR( comm, mpi_errno, myname );

#ifdef MPI_ADI2
    MPIR_ALLOC(*request,(MPI_Request)MPID_PSendAlloc(),
	       comm,MPI_ERR_EXHAUSTED,myname);
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
    shandle->send          = MPIR_IbsendDatatype;

    if (dest != MPI_PROC_NULL) {
	MPIR_ERROR_PUSH(comm);
	/* Allocate space if needed */
	MPIR_CALL_POP(MPI_Pack_size( count, datatype, comm, &psize ),
		      comm,myname);
	MPIR_CALL_POP(MPIR_BsendAlloc( psize, *request, &bufp ),comm,myname);
	/* Information stored in the bsend part by BsendAlloc */
/* 	shandle->shandle.start = bufp;
	shandle->shandle.bytes_as_contig = psize; */
	MPIR_ERROR_POP(comm);
    }
    else 
	/* Rest of dest of MPI_PROC_NULL handled in start */
	shandle->shandle.start = 0;
#else    
    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    MPIR_ALLOC(handleptr,(MPI_Request) MPIR_SBalloc( MPIR_shandles ),
	       comm,MPI_ERR_EXHAUSTED,"Error in MPI_BSEND_INIT");
    *request = handleptr;	       
    handleptr                       = *request;
    MPIR_SET_COOKIE(&handleptr->shandle,MPIR_REQUEST_COOKIE)
    handleptr->type                 = MPIR_SEND;
    if (dest == MPI_PROC_NULL) {
	handleptr->shandle.dest	  = dest;
	MPID_Set_completed( comm->ADIctx, handleptr );
	handleptr->shandle.active = 1;
	handleptr->shandle.bufpos = 0;
	}
    else {
	handleptr->shandle.dest	  = comm->lrank_to_grank[dest];
	MPID_Clr_completed( comm->ADIctx, handleptr );
	handleptr->shandle.active = 0;
	/* We don't use this buffer for bsend, but we will test it 
	   in the wait calls */
	handleptr->shandle.bufpos = 0;
	}
    handleptr->shandle.tag          = tag;
    handleptr->shandle.contextid    = comm->send_context;
    handleptr->shandle.comm         = comm;
    handleptr->shandle.lrank        = comm->local_rank;
    handleptr->shandle.mode         = MPIR_MODE_BUFFERED;
    handleptr->shandle.datatype     = datatype;

    handleptr->shandle.count        = count;
    handleptr->shandle.persistent   = 1;
#ifdef MPID_HAS_HETERO
    handleptr->shandle.msgrep	    = MPIR_MSGREP_SENDER;
#endif
    MPID_Alloc_send_handle( comm->ADIctx, 
			    &((handleptr)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm->ADIctx, 
				 &((handleptr)->shandle.dev_shandle), 1 );

    /* Using pack size should guarentee us enough space.
       We do this as the last option since this is actually creating an
       internal request which is a copy of the external on (except that
       it is a "regular" send) */
    MPI_Pack_size( count, datatype, comm, &psize );
    if ((mpi_errno = 
	MPIR_GetBuffer( psize, handleptr, buf, count, datatype, &bufp ))) 
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_Bsend_init" );
    /* **** */
    handleptr->shandle.bufadd       = bufp;
    /* **** */
    handleptr->shandle.bufadd       = 0;
#endif
    return MPI_SUCCESS;
}
