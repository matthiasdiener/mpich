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

/* #define DEBUG(a) {a} */
#define DEBUG(a)

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_CH_End ANSI_ARGS(( MPID_Device * ));
int MPID_CH_Abort ANSI_ARGS(( MPI_Comm, int, char * ));
void MPID_CH_Version_name ANSI_ARGS(( char * ));

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    Returns a device.  
    This sets up a message-passing device (short/eager/rendezvous protocols)
 */
MPID_Device *MPID_CH_InitMsgPass( argc, argv, short_len, long_len )
int  *argc;
char ***argv;
int  short_len, long_len;
{
    MPID_Device *dev;

    dev = (MPID_Device *)MALLOC( sizeof(MPID_Device) );
    if (!dev) return 0;
    /* The short protocol MUST be for messages no longer than 
       MPID_PKT_MAX_DATA_SIZE since the data must fit within the packet */
    if (short_len < 0) short_len = MPID_PKT_MAX_DATA_SIZE;
    if (long_len < 0)  long_len  = 128000;
    dev->long_len     = short_len;
    dev->vlong_len    = long_len;
    dev->short_msg    = MPID_CH_Short_setup();
#if defined(PI_NO_NSEND) || defined(PI_NO_NRECV) || defined(MPID_USE_BLOCKING)
    dev->long_msg     = MPID_CH_Eagerb_setup();
    dev->vlong_msg    = MPID_CH_Rndvb_setup();
#else
    dev->long_msg     = MPID_CH_Eagern_setup();
    dev->vlong_msg    = MPID_CH_Rndvn_setup();
#endif
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->check_device = MPID_CH_Check_incoming;
    dev->terminate    = MPID_CH_End;
    dev->abort	      = MPID_CH_Abort;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif

    PIiInit( argc, argv );
    DEBUG_PRINT_MSG("Finished init");

    MPID_DO_HETERO(MPID_CH_Init_hetero( argc, argv ));

#ifdef MPID_FLOW_CONTROL
    MPID_FlowSetup();
#endif

    DEBUG_PRINT_MSG("Leaving MPID_CH_InitMsgPass");

    return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_CH_Abort( comm, code, msg )
MPI_Comm comm;
int      code;
char     *msg;
{
    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );
    /* Some systems (e.g., p4) can't accept a (char *)0 message argument. */
    SYexitall( "", code );
    return 0;
}

int MPID_CH_End( dev )
MPID_Device *dev;
{
    DEBUG_PRINT_MSG("Entering MPID_CH_End\n");
    /* Finish off any pending transactions */
    /* MPID_CH_Complete_pending(); */

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
#ifdef MPID_HAS_HETERO /* #HETERO_START# */
    MPID_CH_Hetero_free();
#endif                 /* #HETERO_END# */
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );

#ifdef MPID_FLOW_CONTROL
    MPID_FlowDelete();
#endif

    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    PIiFinish();
    return 0;
}

void MPID_CH_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}

