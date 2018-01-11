/*
 *  $Id: wtime.c,v 1.4 1994/09/21 15:27:28 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: wtime.c,v 1.4 1994/09/21 15:27:28 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
  MPI_Wtime - Returns an elapsed time on the calling processor

  Return value:
  Time in seconds since an arbitrary time in the past.

  Notes:
  This is intended to be a high-resolution, elapsed (or wall) clock.
  See MPI_WTICK to determine the resolution of MPI_WTIME.
@*/
double MPI_Wtime()
{
return MPID_WTIME( MPI_COMM_WORLD->ADIctx );
}
