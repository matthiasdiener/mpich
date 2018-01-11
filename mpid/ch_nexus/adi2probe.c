/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2probe.c,v 1.3 1997/04/01 19:36:19 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */


#include "mpid.h"
#include "dev.h"
#include "mpid_debug.h"
#include "../util/queue.h"

void MPID_Iprobe(struct MPIR_COMMUNICATOR *comm,
		 int tag,
		 int context_id,
		 int src_lrank,
		 int *found,
		 int *error_code,
		 MPI_Status *status)
{
    MPIR_RHANDLE *rhandle = NULL;

    /* Let's sneak a poll in here in case someone sits on an Iprobe */
    nexus_poll();

    MPID_Search_unexpected_queue(src_lrank,
				 tag,
				 context_id,
				 NEXUS_FALSE,
				 &rhandle);
    if (rhandle)
    {
	*found = NEXUS_TRUE;
	status->count = rhandle->count;
	status->MPI_SOURCE = rhandle->s.MPI_SOURCE;
	status->MPI_TAG = rhandle->s.MPI_TAG;
	status->MPI_ERROR = rhandle->s.MPI_ERROR;
    }
    else
    {
	*found = NEXUS_FALSE;
    }
}

void MPID_Probe(struct MPIR_COMMUNICATOR *comm,
		int tag,
		int context_id,
		int src_lrank,
		int *error_code,
		MPI_Status *status)
{
    int found;

    *error_code = 0;

    while(1)
    {
	MPID_Iprobe(comm, tag, context_id, src_lrank, &found, error_code, status);
	if (found || *error_code)
	{
	    break;
	}
	nexus_poll_blocking();
    }
}
