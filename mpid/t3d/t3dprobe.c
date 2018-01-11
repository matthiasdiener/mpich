/*
 *  $Id: t3dprobe.c,v 1.2 1995/06/07 06:39:12 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */
#ifndef lint
static char vcid[] = "$Id";
#endif /* lint */

#include "mpid.h"

/* 
 * ADI Probe functions
 *
 *
 * Interface Description
 * ---------------------
 *
 * This file contains the routines that allow the API to probe for
 * received messages.  Non-blocking and blocking versions are provided.
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *   void T3D_Iprobe(int tag, int source, int context_id, 
 *                      int *found, MPI_Status *status)
 *   void T3D_Probe(int tag, int source, int context_id, 
 *                      MPI_Status *status)
 *      Blocking and non-blocking probe functions.
 */

/****************************************************************************
   T3D_Iprobe

   Description: 
      Implement probe by checking the unexpected receive queue.   
 ****************************************************************************/
int T3D_Iprobe( tag, source, context_id, found, status )
int tag, source, context_id, *found;
MPI_Status *status;
{
    MPIR_RHANDLE *dmpi_unexpected;

#   if defined(MPID_DEBUG_PROBE)
      T3D_Printf("T3D_Iprobe\n");
#   endif


    /* Check the unexpected queue.  Has the message already arrived? */
    DMPI_search_unexpected_queue( source, tag, context_id, 
                                  found, 0, &dmpi_unexpected );

    /* If it wasn't found, drain the device, then search the queue again */
    if (!*found) {
        T3D_Check_incoming( MPID_NOTBLOCKING );
        DMPI_search_unexpected_queue( source, tag, context_id, 
                                      found, 0, &dmpi_unexpected );
    }

    /* If it was found, copy the relevant information to the status
       argument */
    if (*found) {
        register MPIR_RHANDLE *d = dmpi_unexpected;

        /* Copy relevant data to status */
        status->count      = d->dev_rhandle.bytes_as_contig;
        status->MPI_SOURCE = d->source;
        status->MPI_TAG    = d->tag; 
    }
    return (MPI_SUCCESS);
}

/****************************************************************************
   T3D_Probe
 ****************************************************************************/
int T3D_Probe( tag, source, context_id, status )
    int tag, source, context_id;
    MPI_Status *status;
{
    int found;

#   if defined(MPID_DEBUG_PROBE)
      T3D_Printf("T3D_Probe\n");
#   endif


    /* Loop until a message is found */
    while (1) {
        (void)T3D_Iprobe( tag, source, context_id, &found, status );
        if (found) break;
/*        T3D_Check_incoming( MPID_BLOCKING ); */
    }
    return MPI_SUCCESS; 
}
