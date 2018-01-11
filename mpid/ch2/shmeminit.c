/*
 *  $Id: shmeminit.c,v 1.1.1.1 1997/09/17 20:39:30 gropp Exp $
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
#include <stdio.h>

/* #define DEBUG(a) {a} */
#define DEBUG(a)

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/

/* Forward refs */
int MPID_SHMEM_End ANSI_ARGS(( MPID_Device * ));
int MPID_SHMEM_Abort ANSI_ARGS(( MPI_Comm, int, char * ));

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
    dev->short_msg    = MPID_SHMEM_Short_setup();
    dev->long_msg     = MPID_SHMEM_Eagern_setup();
    dev->vlong_msg    = MPID_SHMEM_Rndvn_setup();
    dev->eager        = dev->long_msg;
    dev->rndv         = dev->vlong_msg;
    dev->check_device = MPID_SHMEM_Check_incoming;
    dev->terminate    = MPID_SHMEM_End;
    dev->abort	      = MPID_SHMEM_Abort;
    dev->next	      = 0;

    /* Set the file for Debugging output.  The actual output is controlled
       by MPIDDebugFlag */
#ifdef MPID_DEBUG_ALL
    if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;
#endif

    MPID_SHMEM_init( argc, *argv );
    DEBUG_PRINT_MSG("Finished init");

    DEBUG_PRINT_MSG("Leaving MPID_SHMEM_InitMsgPass");

    return dev;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
int MPID_SHMEM_Abort( comm, code, msg )
MPI_Comm comm;
int      code;
char     *msg;
{
    static int in_abort = 0;

    if (in_abort) exit(code);
    in_abort = 1;

    if (msg) {
	fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, msg );
    }
    else {
	fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
    }
    fflush( stderr );
    fflush( stdout );

    /* This needs to try and kill any generated processes */
    p2p_kill_procs();
    /* Cleanup any "arenas/ipcs/etc" */
    p2p_cleanup();
    exit(code);
    return 0;
}

int MPID_SHMEM_End( dev )
MPID_Device *dev;
{
    DEBUG_PRINT_MSG("Entering MPID_SHMEM_End\n");
    /* Finish off any pending transactions */
    /* MPID_SHMEM_Complete_pending(); */

    if (MPID_GetMsgDebugFlag()) {
	MPID_PrintMsgDebug();
    }
    (dev->short_msg->delete)( dev->short_msg );
    (dev->long_msg->delete)( dev->long_msg );
    (dev->vlong_msg->delete)( dev->vlong_msg );
    FREE( dev );
    /* We should really generate an error or warning message if there 
       are uncompleted operations... */
    MPID_SHMEM_finalize();
    return 0;
}

void MPID_SHMEM_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	     MPIDTRANSPORT );
}

