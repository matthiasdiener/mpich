/*
 *  $Id: getpname.c,v 1.7 1995/12/21 21:57:15 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: getpname.c,v 1.7 1995/12/21 21:57:15 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@C
  MPI_Get_processor_name - Gets the name of the processor

Output Parameters:
. name - A unique specifier for the actual (as opposed to virtual) node. 
. resultlen - Length (in characters) of the name 

Notes:
The name returned should identify a particular piece of hardware; 
the exact format is implementation defined.  This name may or may not
be the same as might be returned by 'gethostname', 'uname', or 'sysinfo'.

.N fortran
@*/
int MPI_Get_processor_name( name, resultlen )
char *name;
int *resultlen;
{
MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, MPI_MAX_PROCESSOR_NAME );
*resultlen = strlen(name);
return MPI_SUCCESS;
}
