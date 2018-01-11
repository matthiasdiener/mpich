/*
 *  $Id: t3ddebug.c,v 1.1 1995/06/07 06:03:18 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3ddebug.c,v 1.1 1995/06/07 06:03:18 bright Exp $";
#endif

#include "mpid.h"

/* 
 * ADI Initialization
 *
 *
 * Interface Description
 * ---------------------
 *
 * This file contains the routines that provide the basic information
 * on the device, and initialize it.  
 *
 * There are no ADI functions which the API can call in this file.
 *
 */


/****************************************************************************
  Include files
 ***************************************************************************/
#include <stdio.h>
#include <varargs.h>


/***************************************************************************
  T3D_Error
 ***************************************************************************/
void T3D_Error(code, fmt, va_alist)
int code;
char *fmt;
va_dcl
{
    va_list ap;

    if (t3d_myid != T3D_UNDEFINED)
        (void)printf("T3D %2d: (error %d) ", t3d_myid, code);
    else
        (void)printf("T3D   : (error %d) ", code);
    va_start(ap);
    (void)vprintf(fmt, ap);
    va_end(ap);
    (void)fflush(stdout);
    T3D_Abort(code);
}


/***************************************************************************
  T3D_Printf
 ***************************************************************************/
void T3D_Printf(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;

    if (t3d_myid != T3D_UNDEFINED)
        (void)printf("T3D %2d: ", t3d_myid);
    else
        (void)printf("T3D   : ");
    va_start(ap);
    (void)vprintf(fmt, ap);
    va_end(ap);
    (void)fflush(stdout);
}
