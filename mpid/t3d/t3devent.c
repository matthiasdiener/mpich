/*
 *  $Id: t3devent.c,v 1.3 1995/06/07 06:23:34 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */
#ifndef lint
static char SCCSid[] = "$Id: t3devent.c,v 1.3 1995/06/07 06:23:34 bright Exp $";
#endif

#include "mpid.h"

/* 
 * Device support for "events" such as receiving un-expected messages.
 *
 *
 * Interface Description
 * ---------------------
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *
 *   int T3D_Cancel( MPIR_COMMON *r )
 *      Cancels a message if possible.  The API does not currently
 *      implement this function.
 */

/***************************************************************************
   T3D_Set_msg_debug_flag
 ***************************************************************************/
static int DebugEventFlag = 0;
void T3D_Set_event_debug_flag( f )
int f;
{
    DebugEventFlag = f;
}

/***************************************************************************
   T3D_Check_device
 ***************************************************************************/
int T3D_Check_device( blocking )
    int blocking;
{
    /*
     * This routine should drain the device and possibly handle
     * pending operations such as non-blocking sends/recvs.  The
     * original code called T3D_Check_incoming to drain incoming
     * messages from the underlying layer.
     */

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_EVENT)
    if (DebugEventFlag) {
        T3D_Printf("T3D_Check_device\n");
    }
#   endif

  T3D_Check_incoming( blocking );

}

/***************************************************************************
   T3D_Cancel

   Description:
      Cancel a message.  This is complicated by the fact that we must
      be able to say that a message HAS been cancelled or completed
      successfully given ONLY the status.
 ***************************************************************************/
int T3D_Cancel( r )
MPIR_COMMON *r;
{
    return MPI_SUCCESS;
}
