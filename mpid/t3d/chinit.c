/*
 *  $Id: chinit.c,v 1.2 1996/06/13 14:52:49 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "flow.h"
#include <stdio.h>
#include "t3dshort.h"
#include "t3dlong.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)


/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

#define MPID_T3D_End   MPID_CH_End
#define MPID_T3D_Abort MPID_T3D_Abort

/* Forward refs */
int MPID_T3D_End ANSI_ARGS(( MPID_Device * ));
int MPID_T3D_Abort ANSI_ARGS(( struct MPIR_COMMUNICATOR *, int, char * ));
void MPID_CH_Version_name ANSI_ARGS(( char * ));
extern void T3D_Init();
extern MPID_CH_Check_incoming();
MPID_Device *ch_t3d_dev;

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *MPID_CH_InitMsgPass(int *argc,char ***argv,int short_len,int long_len )
{
    MPID_Device *dev;


    T3D_Init(*argc,*argv);

    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev)
      return 0;
    /*
       The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet
    */
    if (short_len < 0)
      short_len = T3D_BUFFER_LENGTH;
    if (long_len < 0)
      long_len  = (  T3D_BUFFER_LENGTH
		   * T3D_BUFFER_LENGTH
		   * T3D_BUFFER_LENGTH
		   * T3D_BUFFER_LENGTH);  /* some big number, doesn't matter */
    dev->long_len     = short_len;
    dev->vlong_len    = long_len;
    dev->short_msg    = MPID_T3D_Short_setup();
    dev->long_msg     = MPID_T3D_Long_setup();
    dev->vlong_msg    = dev->long_msg;
    dev->eager        = NULL;
/*  TODO:
    dev->eager        = dev->long_msg;
*/
    dev->rndv         = dev->long_msg;
    dev->check_device = MPID_CH_Check_incoming;
    dev->terminate    = MPID_T3D_End;
    dev->abort	      = MPID_T3D_Abort;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0)
      MPID_DEBUG_FILE = stdout;
#endif

    DEBUG_PRINT_MSG("Finished init");

    DEBUG_PRINT_MSG("Leaving MPID_CH_InitMsgPass");

    ch_t3d_dev = dev;
    return dev;
}

int MPID_T3D_Abort(struct MPIR_COMMUNICATOR *comm, int code, char *msg)
{
    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );
    globalexit(1);
    /* just in case... */
    abort();
    return 0;
}

int MPID_T3D_End(MPID_Device *dev)
{
  extern T3D_Long_Send_Info *blocking_lsi;
  if (dev)
  {
    DEBUG_PRINT_MSG("Entering MPID_CH_End\n");
    
    if (MPID_GetMsgDebugFlag())
    {
      MPID_PrintMsgDebug();
    }
    (dev->short_msg->delete)( dev->short_msg );
    /* TODO:
       (dev->long_msg->delete)( dev->long_msg ); */
    FREE( dev );
  }

  shfree( (void*) t3d_recv_bufs );
  shfree( (void*) t3d_dest_flags );
  shfree( (void*) t3d_lsi );
  shfree( (void*) blocking_lsi );
  shmem_clear_cache_inv();
  
  return 0;
}

void MPID_CH_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}

