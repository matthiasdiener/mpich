/*
 *  $Id: ssend_init.c,v 1.11 1994/09/30 22:11:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "mpisys.h"

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
    int         errno;
    MPI_Request handleptr;

    if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,count) ||
	MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_SEND_TAG(comm,tag) ||
	MPIR_TEST_SEND_RANK(comm,dest)) 
	return MPIR_ERROR(comm, errno, "Error in MPI_SSEND_INIT" );

    *request                        = 
	(MPI_Request) MPIR_SBalloc( MPIR_shandles );
    handleptr                       = *request;
    MPIR_SET_COOKIE(&handleptr->shandle,MPIR_REQUEST_COOKIE)
    handleptr->type                 = MPIR_SEND;
    if (dest == MPI_PROC_NULL)
	handleptr->shandle.dest     = dest;
    else
	handleptr->shandle.dest     = comm->group->lrank_to_grank[dest];
    handleptr->shandle.tag          = tag;
    handleptr->shandle.contextid    = comm->send_context;
    handleptr->shandle.comm         = comm;
    handleptr->shandle.lrank        = 
	comm->local_group->lrank_to_grank[comm->local_group->local_rank];
    handleptr->shandle.mode         = MPIR_MODE_SYNCHRONOUS;
    handleptr->shandle.datatype     = datatype;
    handleptr->shandle.bufadd       = buf;
    handleptr->shandle.count        = count;
    handleptr->shandle.completed    = MPIR_NO;
#ifdef MPID_HAS_HETERO
    handleptr->shandle.msgrep	    = MPIR_MSGREP_SENDER;
#endif
    MPID_Alloc_send_handle( comm->ADIctx, 
			   &((handleptr)->shandle.dev_shandle));
    MPID_Set_send_is_nonblocking( comm->ADIctx, 
				 &((handleptr)->shandle.dev_shandle), 1 );

    return MPI_SUCCESS;
}
