/*
 *  $Id: opfree.c,v 1.15 1997/01/07 01:47:46 gropp Exp $
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

.N NULL

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
    struct MPIR_OP *old;
    static char myname[] = "MPI_OP_FREE";

    TR_PUSH(myname);
    /* Freeing a NULL op returns successfully */
    if (MPIR_TEST_ARG(op))
	return MPIR_ERROR(MPIR_COMM_WORLD,mpi_errno,myname );
    if ( (*op) == MPI_OP_NULL ) {
	TR_POP;
	mpi_errno = MPI_ERR_OP_NULL;
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
    }

    old = MPIR_GET_OP_PTR( *op );
    MPIR_TEST_MPI_OP(*op,old,MPIR_COMM_WORLD,myname);

    /* We can't free permanent objects unless finalize has been called */
    if  ( ( old->permanent == 1 ) && (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_PERM_OP,myname );
    MPIR_CLR_COOKIE(old);
    FREE( old );
    MPIR_RmPointer( *op );

    (*op) = MPI_OP_NULL;

    TR_POP;
    return (MPI_SUCCESS);
}
