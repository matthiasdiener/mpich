/*
 *  $Id: opfree.c,v 1.11 1995/12/21 22:17:17 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: opfree.c,v 1.11 1995/12/21 22:17:17 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@
  MPI_Op_free - Frees a user-defined combination function handle

Input Parameter:
. op - operation (handle) 

Notes:
'op' is set to 'MPI_OP_NULL' on exit.

.N fortran
@*/
int MPI_Op_free( op )
MPI_Op  *op;
{
  int mpi_errno;
  /* Freeing a NULL op returns successfully */
  if (MPIR_TEST_ARG(op))
      return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,"Error in MPI_OP_FREE" );
  if ( (*op) == MPI_OP_NULL )
	return (MPI_SUCCESS);

  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*op)->permanent == 1 ) && (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_PERM_OP,
					  "Error in MPI_OP_FREE" );

  /* Free the op */
  MPIR_SET_COOKIE( *op, 0 )
  FREE( (*op) );
  (*op) = MPI_OP_NULL;

  return (MPI_SUCCESS);
}
