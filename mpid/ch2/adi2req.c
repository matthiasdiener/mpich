/*
 *  $Id: adi2req.c,v 1.2 1996/06/26 19:27:47 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "reqalloc.h"
#include "sendq.h"	/* For MPIR_FORGET_SEND */

/***************************************************************************
 *
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * This file handles freed but not completed requests
 *
 ***************************************************************************/

void MPID_Request_free( request )
MPI_Request request;
{
    /* Allow only one member for now */
    while (MPID_devset->req_pending) {
	MPI_Request rq = MPID_devset->req_pending;
	int         mpi_errno = MPI_SUCCESS;

	switch (rq->handle_type) {
	case MPIR_SEND:
	    if (MPID_SendIcomplete( request, &mpi_errno )) {
	        MPIR_FORGET_SEND( &rq->shandle );
		MPID_SendFree( rq );
		MPID_devset->req_pending = 0;
	    }
	    break;
	case MPIR_RECV:
	    if (MPID_RecvIcomplete( request, (MPI_Status *)0, &mpi_errno )) {
		MPID_RecvFree( rq );
		MPID_devset->req_pending = 0;
	    }
	    break;
	case MPIR_PERSISTENT_SEND:
	    printf( "Not done - persistent_send_free\n" );
	    break;
	case MPIR_PERSISTENT_RECV:
	    printf( "Not done - persistent_send_free\n" );
	    break;
	}
	MPID_DeviceCheck( MPID_NOTBLOCKING );
    }
    /* Add to devset's request list */
    MPID_devset->req_pending = request;
}
