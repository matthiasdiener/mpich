/*
 *  $Id: keyval_free.c,v 1.14 1996/06/07 15:08:25 gropp Exp $
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
#include "attr.h"

/*@

MPI_Keyval_free - Frees attribute key for communicator cache attribute

Input Parameter:
. keyval - Frees the integer key value (integer) 

Note:
Key values are global (they can be used with any and all communicators)

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
.N MPI_ERR_PERM_KEY

.seealso: MPI_Keyval_create
@*/
int MPI_Keyval_free ( keyval )
int *keyval;
{
  int mpi_errno;
  MPIR_Attr_key *attr_key;

  if ( MPIR_TEST_ARG(keyval) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_KEYVAL_FREE");

#ifdef INT_LT_POINTER
  attr_key = (MPIR_Attr_key *)MPIR_ToPointer( *keyval );
#else
  attr_key = (MPIR_Attr_key *)*keyval;
#endif
  if ( (*keyval) != MPI_KEYVAL_INVALID ) {
	if ( (attr_key->permanent == 1) && (MPIR_Has_been_initialized == 1) ){
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_PERM_KEY, 
		"Can't free permanent attribute key in MPI_KEYVAL_FREE" );
	}
	if (attr_key->ref_count <= 1) {
	    MPIR_SET_COOKIE(attr_key,0);
	    FREE ( attr_key );
#ifdef INT_LT_POINTER
	    MPIR_RmPointer( *keyval );
#endif
	  }
	else {
	  attr_key->ref_count--;
#ifdef FOO
	  /* Debugging only */
	  if (MPIR_Has_been_initialized != 1) 
	      PRINTF( "attr_key count is %d\n", attr_key->ref_count );
#endif
	}
	(*keyval) = MPI_KEYVAL_INVALID;
  }
  return (MPI_SUCCESS);
}
