/*
 *  $Id: create_recv.c,v 1.17 1996/01/11 18:30:13 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

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
    MPI_Request handleptr;

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_RECV_TAG(comm,tag) ||
	MPIR_TEST_RECV_RANK(comm,source)) 
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_RECV_INIT" );

    /* See MPI_TYPE_FREE.  A free can not happen while the datatype may
       be in use.  Thus, a nonblocking operation increments the
       reference count */
    MPIR_GET_REAL_DATATYPE(datatype)
    datatype->ref_count++;
    *request			   = 
	(MPI_Request) MPIR_SBalloc( MPIR_rhandles );
    handleptr			   = *request;
    MPIR_SET_COOKIE(&handleptr->rhandle,MPIR_REQUEST_COOKIE)
    handleptr->type		   = MPIR_RECV;
#ifdef MPID_NEEDS_WORLD_SRC_INDICES
    handleptr->rhandle.source = (source >= 0) ? (comm->lrank_to_grank[source])
	                                    : source;
#else
    handleptr->rhandle.source      = source;
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

    return MPI_SUCCESS;
}
