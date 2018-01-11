/*
 *  $Id: keyval_free.c,v 1.19 1997/02/18 23:06:13 gropp Exp $
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
  static char myname[] = "MPI_KEYVAL_FREE";

  if ( MPIR_TEST_ARG(keyval) )
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  if (*keyval == MPI_KEYVAL_INVALID) {
      /* Can't free an invalid keyval */
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );
  }
  attr_key = MPIR_GET_KEYVAL_PTR( *keyval );
  MPIR_TEST_MPI_KEYVAL(*keyval,attr_key,MPIR_COMM_WORLD,myname);

  if ( (attr_key->permanent == 1) && (MPIR_Has_been_initialized == 1) ){
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_PERM_KEY, 
	 "Can't free permanent attribute key in MPI_KEYVAL_FREE" );
  }
  if (attr_key->ref_count <= 1) {
      MPIR_CLR_COOKIE(attr_key);
      FREE ( attr_key );
      MPIR_RmPointer( *keyval );
  }
  else {
      MPIR_REF_DECR(attr_key);
#ifdef FOO
      /* Debugging only */
      if (MPIR_Has_been_initialized != 1) 
	  PRINTF( "attr_key count is %d\n", attr_key->ref_count );
#endif
  }
  (*keyval) = MPI_KEYVAL_INVALID;
  
  return (MPI_SUCCESS);
}
