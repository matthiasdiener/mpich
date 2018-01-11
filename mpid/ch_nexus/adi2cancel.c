/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2cancel.c,v 1.2 1997/04/01 19:36:17 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
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
