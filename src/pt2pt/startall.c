/*
 *  $Id: startall.c,v 1.4 1994/07/13 04:04:29 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: startall.c,v 1.4 1994/07/13 04:04:29 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
  MPI_Startall - Starts a collection of requests 

Input Parameters:
. count - list length (integer) 
. array_of_requests - array of requests (array of handle) 
@*/
int MPI_Startall( count, array_of_requests )
int count;
MPI_Request array_of_requests[];
{
int i;
int errno;
for (i=0; i<count; i++)
    if (errno = MPI_Start( array_of_requests + i )) return errno;

return MPI_SUCCESS;
}
