/*
 *  $Id: requestc2f.c,v 1.2 1998/02/02 22:53:32 gropp Exp $
 *
 *  (C) 1997 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Request_c2f - Convert a C request to a Fortran request

Input Parameters:
. c_request - Request value in C (handle)

Output Value:
. f_request - Status value in Fortran (Integer)
  
.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
@*/
MPI_Fint MPI_Request_c2f( c_request )
MPI_Request  c_request;
{
    MPI_Fint f_request;
    
    if (c_request == MPI_REQUEST_NULL) return 0;

    /* If we've registered this request, return the current value */
    if (c_request->chandle.self_index)
	return c_request->chandle.self_index;
    f_request = MPIR_FromPointer( c_request );
    c_request->chandle.self_index = f_request;
    return f_request;
}
