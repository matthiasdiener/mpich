/*
 *  $Id: wtick.c,v 1.6 1996/04/11 20:31:04 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpid_time.h"
#endif

/*@
  MPI_Wtick - Returns the resolution of MPI_Wtime

  Return value:
  Time in seconds of resolution of MPI_Wtime

  Notes for Fortran:
  This is a function, declared as 'DOUBLE PRECISION MPI_WTICK()' in Fortran.
  
@*/
double MPI_Wtick()
{
#ifdef MPI_ADI2
    double t1;
    MPID_Wtick( &t1 );
    return t1;
#else
    return MPID_WTICK( MPI_COMM_WORLD->ADIctx );
#endif
}
