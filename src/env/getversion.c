/*
 *  $Id: getversion.c,v 1.3 1998/04/28 21:09:00 swider Exp $
 *
 *  (C) 1997 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Get_version - Gets the version of MPI

Output Parameters:
+ version - Major version of MPI (1 or 2)
- subversion - Minor version of MPI.  

Notes:
The defined values 'MPI_VERSION' and 'MPI_SUBVERSION' contain the same 
information.  This routine allows you to check that the library matches the 
version specified in the 'mpi.h' and 'mpif.h' files.

.N fortran
@*/
int MPI_Get_version( version, subversion )
int *version;
int *subversion;
{
    *version    = MPI_VERSION;
    *subversion = MPI_SUBVERSION;
    return MPI_SUCCESS;
}
