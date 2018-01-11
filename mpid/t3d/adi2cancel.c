/*
 *  $Id: adi2cancel.c,v 1.2 1996/12/03 02:51:46 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "../util/queue.h"

/*
 * This file contains the routines to handle canceling a message
 *
 * Note: for now, cancel will probably only work on unmatched receives.
 * However, this code provides the hooks for supporting more complete
 * cancel implementations.
 */

void MPID_SendCancel( request, error_code )
MPI_Request request;
int         *error_code;
{
    *error_code = MPI_SUCCESS;
    /* This isn't really correct */
}

void MPID_RecvCancel( request, error_code )
MPI_Request request;
int         *error_code;
{
    MPIR_RHANDLE *rhandle = (MPIR_RHANDLE *)request;

    /* First, try to find in pending receives */
    if (MPID_Dequeue( &MPID_recvs.posted, rhandle ) == 0) {
	/* Mark the request as cancelled */
	rhandle->s.MPI_TAG = MPIR_MSG_CANCELLED;
	/* Mark it as complete */
	rhandle->is_complete = 1;
	/* Should we call finish to free any space?  cancel? */

	/* If request is a persistent one, we need to mark it as inactive
	   as well */
	if (rhandle->handle_type == MPIR_PERSISTENT_RECV) {
	    MPIR_PRHANDLE *prhandle = (MPIR_PRHANDLE *)request;
	    prhandle->active = 0;
	}
    }
    else {
	/* Mark the request as not cancelled */
	/* tag is already set >= 0 as part of receive */
	/* rhandle->s.tag = 0; */
	;
        /* What to do about an inactive persistent receive? */
    }

    /* In the case of a partly completed rendezvous receive, we might
       want to do something */
    *error_code = MPI_SUCCESS;
}


