/*
 *  $Id: errclass.c,v 1.1.1.1 1997/09/17 20:41:53 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

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
