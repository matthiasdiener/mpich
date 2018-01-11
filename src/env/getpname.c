/*
 *  $Id: getpname.c,v 1.8 1996/04/11 20:29:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

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
#ifdef MPI_ADI2
    MPID_Node_name( name, MPI_MAX_PROCESSOR_NAME );
#else
    MPID_NODE_NAME( MPI_COMM_WORLD->ADIctx, name, MPI_MAX_PROCESSOR_NAME );
#endif
    *resultlen = strlen(name);
    return MPI_SUCCESS;
}
