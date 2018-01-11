#define PI_NO_NSEND
#define PI_NO_NRECV


/*
 *  $Id: chprobe.c,v 1.8 1994/10/24 22:03:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id";
#endif /* lint */

#include "mpid.h"

/*
   Implement probe by checking the unexpected receive queue.

 */
int MPID_N3_Iprobe( tag, source, context_id, found, status )
int tag, source, context_id, *found;
MPI_Status *status;
{
MPIR_RHANDLE *dmpi_unexpected;

/* At this time, we check to see if the message has already been received */
DMPI_search_unexpected_queue( source, tag, context_id, 
			      found, 0, &dmpi_unexpected );
if (!*found) {
    /* If nothing there, check for incoming messages.... */
    /* MPID_check_incoming should return whether it found anything when
       called in the non-blocking mode... */
    MPID_N3_check_incoming( MPID_NOTBLOCKING );
    DMPI_search_unexpected_queue( source, tag, context_id, 
				  found, 0, &dmpi_unexpected );
    }
if (*found) {
    register MPIR_RHANDLE *d = dmpi_unexpected;

    /* Copy relevant data to status */
    status->count	   = d->dev_rhandle.bytes_as_contig;
    status->MPI_SOURCE	   = d->source;
    status->MPI_TAG	   = d->tag; 
    }
return MPI_SUCCESS;
}


int MPID_N3_Probe( tag, source, context_id, status )
int tag, source, context_id;
MPI_Status *status;
{
int found;
while (1) {
    MPID_N3_Iprobe( tag, source, context_id, &found, status );
    if (found) break;
    /* Wait for a message */
    MPID_N3_check_incoming( MPID_BLOCKING );
    }
return MPI_SUCCESS; /* or make routine void ... */    
}
