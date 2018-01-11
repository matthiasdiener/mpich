/*
 *  $Id: wtick.c,v 1.2 1998/01/29 14:27:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "mpid_time.h"

/*@
  MPI_Wtick - Returns the resolution of MPI_Wtime

  Return value:
  Time in seconds of resolution of MPI_Wtime

  Notes for Fortran:
  This is a function, declared as 'DOUBLE PRECISION MPI_WTICK()' in Fortran.
  
@*/
double MPI_Wtick()
{
    double t1;
    MPID_Wtick( &t1 );
    return t1;
}
