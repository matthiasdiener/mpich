/*
 *  $Id: opcreate.c,v 1.13 1997/01/07 01:47:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif
#include "mpiops.h"

/*@
  MPI_Op_create - Creates a user-defined combination function handle

Input Parameters:
. function - user defined function (function) 
. commute -  true if commutative;  false otherwise. 

Output Parameter:
. op - operation (handle) 

.N fortran

.N collops

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Op_free
@*/
int MPI_Op_create( function, commute, op )
MPI_User_function *function;
int               commute;
MPI_Op            *op;
{
    struct MPIR_OP *new;
    MPIR_ALLOC(new,NEW( struct MPIR_OP ),MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
	       "Out of space in MPI_OP_CREATE");
    MPIR_SET_COOKIE(new,MPIR_OP_COOKIE)
    new->commute   = commute;
    new->op	   = function;
    new->permanent = 0;
    *op = (MPI_Op)MPIR_FromPointer( new );
    return (MPI_SUCCESS);
}
