/*
 *  $Id: opcreate.c,v 1.11 1996/04/12 15:39:46 gropp Exp $
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
  MPI_Op newop;

  MPIR_ALLOC(newop,NEW( struct MPIR_OP ),MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
                      "Out of space in MPI_OP_CREATE");
  MPIR_Op_setup( function, commute, 0, newop );
  (*op)            = newop;
  
  return (MPI_SUCCESS);
}
