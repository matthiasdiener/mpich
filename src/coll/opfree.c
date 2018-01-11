/*
 *  $Id: opfree.c,v 1.12 1996/04/12 15:40:11 gropp Exp $
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
  MPI_Op_free - Frees a user-defined combination function handle

Input Parameter:
. op - operation (handle) 

Notes:
'op' is set to 'MPI_OP_NULL' on exit.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
.N MPI_ERR_PERM_OP

.seealso: MPI_Op_create
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
