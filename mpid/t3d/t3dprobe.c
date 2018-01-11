/*
 *  $Id: t3dprobe.c,v 1.3 1995/06/11 05:42:57 bright Exp $
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

/***************************************************************************
   T3D_Set_probe_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;
void T3D_Set_probe_debug_flag( f )
int f;
{
    DebugFlag = f;
}

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
    T3D_PKT_T    *pkt;

#   if defined(MPID_DEBUG_PROBE) || defined(MPID_DEBUG_ALL)
    if (DebugFlag)
      T3D_Printf("T3D_Iprobe\n");
#   endif

    
    /* Check the unexpected queue.  Has the message already arrived? */
    DMPI_search_unexpected_queue( source, tag, context_id, 
                                  found, 0, &dmpi_unexpected );

    /* If it wasn't found, drain the device, then search the queue again */
    if (!*found) {

      if ( source != MPI_ANY_SOURCE ) {
	int tagmask;

	if ( tag == MPI_ANY_TAG ) 
	  tag = tagmask = 0;
	else
	  tagmask = ~0;

	if ( (t3d_recv_bufs[source].head.status == T3D_BUF_IN_USE) &&
	     (t3d_recv_bufs[source].head.context_id == context_id) &&
	    ((t3d_recv_bufs[source].head.tag & tagmask) == tag )      ) {

	  *found             = 1;
	  status->count      = t3d_recv_bufs[source].head.len;
	  status->MPI_SOURCE = source;
	  status->MPI_TAG    = t3d_recv_bufs[source].head.tag;

#       if defined(MPID_DEBUG_PROBE) || defined(MPID_DEBUG_ALL)
	  if ( DebugFlag ) {
	    T3D_Printf("  Found message with\n");
	    T3D_Printf("   count  = %d\n",status->count);
	    T3D_Printf("   source = %d\n",status->MPI_SOURCE);
	    T3D_Printf("   tag    = %d\n",status->MPI_TAG);
	  }
#       endif

	  return MPI_SUCCESS;
	}
      }
      else {
	T3D_Check_incoming( MPID_NOTBLOCKING );
	DMPI_search_unexpected_queue( source, tag, context_id, 
				     found, 0, &dmpi_unexpected );
      }
    }

    /* If it was found, copy the relevant information to the status
       argument */
    if (*found) {

        /* Copy relevant data to status */
        status->count      = dmpi_unexpected->totallen;
        status->MPI_SOURCE = dmpi_unexpected->source;
        status->MPI_TAG    = dmpi_unexpected->tag;

#       if defined(MPID_DEBUG_PROBE) || defined(MPID_DEBUG_ALL)
	if ( DebugFlag ) {
	  T3D_Printf("  Found message with\n");
	  T3D_Printf("   count  = %d\n",status->count);
	  T3D_Printf("   source = %d\n",status->MPI_SOURCE);
	  T3D_Printf("   tag    = %d\n",status->MPI_TAG);
	}
#       endif
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
    int found = 0;

#   if defined(MPID_DEBUG_PROBE) || defined(MPID_DEBUG_ALL)
    if ( DebugFlag )
      T3D_Printf("T3D_Probe\n");
#   endif


    /* Loop until a message is found */
    while (! found )
        T3D_Iprobe( tag, source, context_id, &found, status );

    return MPI_SUCCESS; 
}
