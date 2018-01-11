/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/* This is a special stand-alone error handler */
#include <stdio.h>

#include "mpid.h"
/* #include "mpisys.h" */
#include "queue.h"

/* 
   This calls the user-specified error handler.  If that handler returns,
   we return the error code 
 */
int MPIR_Error( comm, code, string, file, line )
MPI_Comm  comm;
int       code, line;
char     *string, *file;
{
  fprintf( stderr, "%d - %s\n", MPID_MyWorldRank, 
          string ? string : "<NO ERROR MESSAGE>" );
  MPID_Abort( comm, code, (char *)0, (char *)0 );
  return (code);
}
