/*
 *  $Id: wtick.c,v 1.4 1994/09/21 15:27:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: wtick.c,v 1.4 1994/09/21 15:27:26 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"

/*@
  MPI_Wtick - Returns the resolution of MPI_Wtime

  Return value:
  Time in seconds of resolution of MPI_Wtime
@*/
double MPI_Wtick()
{
return MPID_WTICK( MPI_COMM_WORLD->ADIctx );
}
