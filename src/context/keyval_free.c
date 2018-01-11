/*
 *  $Id: keyval_free.c,v 1.10 1994/12/11 16:54:55 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Keyval_free - Frees attribute key for communicator cache attribute

Input Parameter:
. keyval - Frees the integer key value (integer) 

Note:
Key values are global (they can be used with any and all communicators)
@*/
int MPI_Keyval_free ( keyval )
int *keyval;
{
  int errno;
  MPIR_Attr_key *attr_key;

  if ( MPIR_TEST_ARG(keyval))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_KEYVAL_FREE");

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
	/* Key reference counts are NOT being incremented when a communicator
	   is copied.  For now, we'll just waste keyvals ... */
	if (/* 0 && */ attr_key->ref_count <= 1) {
	  FREE ( attr_key );
#ifdef INT_LT_POINTER
	  MPIR_RmPointer( *keyval );
#endif
	  }
	else
	  attr_key->ref_count--;
	(*keyval) = MPI_KEYVAL_INVALID;
  }
  return (MPI_SUCCESS);
}
