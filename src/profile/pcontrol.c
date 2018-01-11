/*
 *  $Id: pcontrol.c,v 1.1.1.1 1997/09/17 20:42:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#if defined(__STDC__) || defined(HAVE_PROTOTYPES)
#ifdef HAVE_NO_C_CONST
int MPI_Pcontrol( int level, ... )
#else
int MPI_Pcontrol( const int level, ... )
#endif
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
