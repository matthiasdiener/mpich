/*
 *  $Id: wtime.c,v 1.6 1996/04/11 20:31:11 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpid_time.h"
#else
#include "mpisys.h"
#endif

/*@
  MPI_Wtime - Returns an elapsed time on the calling processor

  Return value:
  Time in seconds since an arbitrary time in the past.

  Notes:
  This is intended to be a high-resolution, elapsed (or wall) clock.
  See 'MPI_WTICK' to determine the resolution of 'MPI_WTIME'.
  If the attribute 'MPI_WTIME_IS_GLOBAL' is defined and true, then the
  value is synchronized across all processes in 'MPI_COMM_WORLD'.  

  Notes for Fortran:
  This is a function, declared as 'DOUBLE PRECISION MPI_WTIME()' in Fortran.

.see also: MPI_Wtick, MPI_Attr_get
@*/
double MPI_Wtime()
{
#ifdef MPI_ADI2
    double t1;
    MPID_Wtime( &t1 );
    return t1;
#else
    return MPID_WTIME( MPI_COMM_WORLD->ADIctx );
#endif
}
