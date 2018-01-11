/*
 *  $Id: chchkdev.c,v 1.3 1996/07/17 18:04:59 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include "mpid.h"
#include "mpiddev.h"
#include "flow.h"
#include "../util/queue.h"
#include "t3dpriv.h"
#include "t3dlong.h"

/***************************************************************************/
/* This is one of the main routines.  It checks for incoming messages and  */
/* dispatches them.  There is another such look in MPID_CH_blocking_recv   */
/* which is optimized for the important case of blocking receives for a    */
/* particular message.                                                     */
/***************************************************************************/

/* 
   Check for incoming messages.
   Input Parameter:
   .   is_blocking - true if this routine should block until a message is
       available
   
   Returns -1 if nonblocking and no messages pending.  Otherwise 
   returns error code (MPI_SUCCESS == 0 for success)
   
   This routine makes use of a single dispatch routine to handle all
   incoming messages.  This makes the code a little lengthy, but each
   piece is relatively simple.
   
   This is the message-passing version.  The shared-memory version is
   in chchkshdev.c .
*/    

int MPID_CH_Check_incoming( dev, is_blocking )
MPID_Device        *dev;
MPID_BLOCKING_TYPE is_blocking;
{
    volatile T3D_PKT_T   *pkt;
    int          from_grank;
    MPIR_RHANDLE *rhandle;
    int          is_posted;
    int          err = MPI_SUCCESS;
    int          i=0;
    static int   nLastUsed=-1;

    extern int   MPID_MyWorldSize;
    extern int   MPID_MyWorldRank;
    
    DEBUG_PRINT_MSG("Entering check_incoming");
    
    if (MPID_NOTBLOCKING == is_blocking)
    {
      err = -1;
      MPID_T3D_Progress_Long_Sends();
      for (i=0; i < MPID_MyWorldSize; i++)
      {
	nLastUsed = (nLastUsed + 1) % MPID_MyWorldSize;
	if (T3D_BUF_IN_USE == t3d_recv_bufs[nLastUsed].head.status)
	  break;
      }
    }
    else
    {
      int loops=0;
      do
      {
	if ((loops % MPID_MyWorldSize) == 0)
	  MPID_T3D_Progress_Long_Sends();
	nLastUsed = (nLastUsed + 1) % MPID_MyWorldSize;
	loops++;
      } while (T3D_BUF_IN_USE != t3d_recv_bufs[nLastUsed].head.status);
    }
    
    if (T3D_BUF_IN_USE == t3d_recv_bufs[nLastUsed].head.status)
    {
      pkt = &(t3d_recv_bufs[nLastUsed]);
      DEBUG_PRINT_PKT("R received message",&pkt);
      MPID_Msg_arrived( pkt->head.lrank, pkt->head.tag, pkt->head.context_id, 
		       &rhandle, &is_posted );
      from_grank = pkt->head.source;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
      if (MPID_DebugFlag)
      {
        FPRINTF( MPID_DEBUG_FILE, "[%d]R msg was %s (%s:%d)\n", 
                MPID_MyWorldRank, 
                is_posted ? "posted" : "unexpected", __FILE__, __LINE__ );
      }
#endif                  /* #DEBUG_END# */
      if (is_posted)
      {
        switch (pkt->head.mode)
	{
	case T3D_PKT_SHORT:
	  err = (*dev->short_msg->recv)( rhandle, from_grank, pkt );
	  break;
	case T3D_PKT_LONG:
	  err = (*dev->long_msg->recv)(rhandle, from_grank, pkt);
	  break;
	default:
          fprintf( stderr, 
                  "[%d] Internal error: msg packet discarded (%s:%d)\n",
                  MPID_MyWorldRank, __FILE__, __LINE__ );
	  break;
	}
      }
      else
      {
	switch (pkt->head.mode)
	{
	case T3D_PKT_SHORT:
	  err = (*dev->short_msg->unex)( rhandle, from_grank, pkt );
	  break;
	case T3D_PKT_LONG:
	  err = (*dev->long_msg->unex)( rhandle, from_grank, pkt );
	  break;
	default:
          fprintf( stderr, 
                  "[%d] Internal error: msg packet discarded (%s:%d)\n",
                  MPID_MyWorldRank, __FILE__, __LINE__ );
	  break;
	}
      }
    }
    else
      DEBUG_PRINT_MSG("Leaving check_incoming (no messages)");
    DEBUG_PRINT_MSG("Exiting check_incoming");
    
    return err;
}
