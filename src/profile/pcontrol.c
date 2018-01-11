/*
 *  $Id: pcontrol.c,v 1.7 1996/04/12 15:57:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifdef __STDC__
int MPI_Pcontrol( const int level, ... )
{
    return MPI_SUCCESS;
}
#else
/*@
  MPI_Pcontrol - Controls profiling

  Input Parameters:
. level - Profiling level 

  Notes:
  This routine provides a common interface for profiling control.  The
  interpretation of 'level' and any other arguments is left to the
  profiling library.

.N fortran
@*/
int MPI_Pcontrol( level )
int level;
{
    return MPI_SUCCESS;
}
#endif
