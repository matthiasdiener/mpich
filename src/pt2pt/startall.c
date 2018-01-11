/*
 *  $Id: startall.c,v 1.5 1994/12/15 17:23:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: startall.c,v 1.5 1994/12/15 17:23:31 gropp Exp $";
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
int mpi_errno;
for (i=0; i<count; i++)
    if (mpi_errno = MPI_Start( array_of_requests + i )) return mpi_errno;

return MPI_SUCCESS;
}
