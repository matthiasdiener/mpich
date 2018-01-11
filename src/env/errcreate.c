/*
 *  $Id: errcreate.c,v 1.8 1996/04/11 20:27:58 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "sbcnst2.h"
#define MPIR_SBalloc MPID_SBalloc
#else
#include "mpisys.h"
#endif

/*@
  MPI_Errhandler_create - Creates an MPI-style errorhandler

Input Parameter:
. function - user defined error handling procedure 

Output Parameter:
. errhandler - MPI error handler (handle) 

Notes:
The MPI Standard states that an implementation may make the output value 
(errhandler) simply the address of the function.  However, the action of 
'MPI_Errhandler_free' makes this impossible, since it is required to set the
value of the argument to 'MPI_ERRHANDLER_NULL'.  In addition, the actual
error handler must remain until all communicators that use it are 
freed.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_EXHAUSTED
@*/
int MPI_Errhandler_create( function, errhandler )
MPI_Handler_function *function;
MPI_Errhandler       *errhandler;
{
    MPI_Errhandler new;

    MPIR_ALLOC(new,(MPI_Errhandler) MPIR_SBalloc( MPIR_errhandlers ),
	       MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			   "Error in MPI_ERRHANDLER_CREATE" );

    MPIR_SET_COOKIE(new,MPIR_ERRHANDLER_COOKIE);
    new->routine   = function;
    new->ref_count = 1;

    *errhandler = new;
    return MPI_SUCCESS;
}
