/*
 *  $Id: errclass.c,v 1.4 1995/12/21 21:56:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: errclass.c,v 1.4 1995/12/21 21:56:41 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"
#include "mpisys.h"

/*@
   MPI_Error_class - Converts an error code into an error class

Input Parameter:
. errorcode - Error code returned by an MPI routine 

Output Parameter:
. errorclass - Error class associated with 'errorcode'

.N fortran
@*/
int MPI_Error_class( errorcode, errorclass )
int errorcode, *errorclass;
{
*errorclass = errorcode & 0xff;
return MPI_SUCCESS;
}
