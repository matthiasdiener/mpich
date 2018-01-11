/*
 *  $Id: bsend_init.c,v 1.19 1995/03/05 20:15:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "mpisys.h"

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
    MPI_Request handleptr;
    int         psize;
    void        *bufp;

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || 
	MPIR_TEST_SEND_RANK(comm,dest) || MPIR_TEST_SEND_TAG(comm,tag))
	MPIR_ERROR( comm, mpi_errno, "Error in MPI_Bsend_init" );
    
    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    datatype->ref_count++;
    *request                        = 
	(MPI_Request) MPIR_SBalloc( MPIR_shandles );
    handleptr                       = *request;
    MPIR_SET_COOKIE(&handleptr->shandle,MPIR_REQUEST_COOKIE)
    handleptr->type                 = MPIR_SEND;
    if (dest == MPI_PROC_NULL) {
	handleptr->shandle.dest	  = dest;
	MPID_Set_completed( comm->ADIctx, handleptr );
	handleptr->shandle.active = 1;
	}
    else {
	handleptr->shandle.dest	  = comm->group->lrank_to_grank[dest];
	MPID_Clr_completed( comm->ADIctx, handleptr );
	handleptr->shandle.active = 0;
	}
    handleptr->shandle.tag          = tag;
    handleptr->shandle.contextid    = comm->send_context;
    handleptr->shandle.comm         = comm;
    handleptr->shandle.lrank        = comm->local_group->local_rank;
    handleptr->shandle.mode         = MPIR_MODE_BUFFERED;
    handleptr->shandle.datatype     = datatype;

    /* Using pack size should guarentee us enough space */
    MPI_Pack_size( count, datatype, comm, &psize );
    if (mpi_errno = 
	MPIR_GetBuffer( psize, handleptr, buf, count, datatype, &bufp )) 
	MPIR_ERROR( comm, mpi_errno, "Error in MPI_Bsend_init" );

    handleptr->shandle.bufadd       = bufp;
    handleptr->shandle.count        = count;
    handleptr->shandle.persistent   = 1;
#ifdef MPID_HAS_HETERO
    handleptr->shandle.msgrep	    = MPIR_MSGREP_SENDER;
#endif
    MPID_Alloc_send_handle( comm->ADIctx, 
			    &((handleptr)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm->ADIctx, 
				 &((handleptr)->shandle.dev_shandle), 1 );

    return MPI_SUCCESS;
}
