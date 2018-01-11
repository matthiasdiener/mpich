/*
 *  $Id: t3dsync.c,v 1.1 1995/06/07 06:04:17 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3dsync.c,v 1.1 1995/06/07 06:04:17 bright Exp $";
#endif

#include "mpid.h"

/* 
 * ADI sycnronous routines
 *
 *
 * Interface Description
 * ---------------------
 *
 * 
 * This file contains routines to keep track of synchronous send messages;
 * they must be explicitly acknowledged.
 *
 * Currently, there are no ADI functions provided to the API.
 *
 */

/***************************************************************************
   T3D_Set_sync_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;
void T3D_Set_sync_debug_flag( f )
int f;
{
    DebugFlag = f;
}
