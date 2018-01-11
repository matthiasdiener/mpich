/*
 *  $Id: opcreate.c,v 1.10 1995/12/21 22:17:13 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: opcreate.c,v 1.10 1995/12/21 22:17:13 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@
  MPI_Op_create - Creates a user-defined combination function handle

Input Parameters:
. function - user defined function (function) 
. commute -  true if commutative;  false otherwise. 

Output Parameter:
. op - operation (handle) 

.N fortran
@*/
int MPI_Op_create( function, commute, op )
MPI_User_function *function;
int               commute;
MPI_Op            *op;
{
  MPI_Op newop;

  newop            = NEW( struct MPIR_OP );
  if (!newop) 
    return MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
                      "Out of space in MPI_OP_CREATE");
  MPIR_Op_setup( function, commute, 0, newop );
  (*op)            = newop;
  
  return (MPI_SUCCESS);
}
