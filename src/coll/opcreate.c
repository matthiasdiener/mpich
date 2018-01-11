/*
 *  $Id: opcreate.c,v 1.8 1994/11/21 20:41:24 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: opcreate.c,v 1.8 1994/11/21 20:41:24 gropp Exp $";
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
  MPIR_SET_COOKIE(newop,MPIR_OP_COOKIE)
  newop->commute   = commute;
  newop->op        = function;
  newop->permanent = 0;
  (*op)            = newop;
  
  return (MPI_SUCCESS);
}
