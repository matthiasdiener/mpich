/*
 *  $Id: pcontrol.c,v 1.5 1994/07/13 15:51:04 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: pcontrol.c,v 1.5 1994/07/13 15:51:04 lusk Exp $";
#endif

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
  interpretation of "level" and any other arguments is left to the
  profiling library.
@*/
int MPI_Pcontrol( level )
int level;
{
return MPI_SUCCESS;
}
#endif
