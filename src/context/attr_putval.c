/*
 *  $Id: attr_putval.c,v 1.16 1994/08/12 20:39:33 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Attr_put - Stores attribute value associated with a key

Input Parameters:
. comm - communicator to which attribute will be attached (handle) 
. keyval - key value, as returned by  MPI_KEYVAL_CREATE (integer) 
. attribute_val - attribute value 

Notes:
Values of the permanent attributes MPI_TAG_UB, MPI_HOST, and MPI_IO may not 
be changed. 

The type of the attribute value depends on whether C or Fortran is being used.
In C, an attribute value is a pointer (void *); in Fortran, it is a single 
integer (NOT a pointer, since Fortran has no pointers and their are systems 
for which a pointer does not fit in an integer (e.g., any > 32 bit address 
system that uses 64 bits for Fortran DOUBLE PRECISION).
@*/
int MPI_Attr_put ( comm, keyval, attr_value )
MPI_Comm comm;
int      keyval;
void     *attr_value;
{
  MPIR_HBT_node *attr;
  MPIR_Attr_key *attr_key;
  int errno = MPI_SUCCESS;

#ifdef INT_LT_POINTER
  attr_key = (MPIR_Attr_key *)MPIR_ToPointer( keyval );
#else
  attr_key = (MPIR_Attr_key *)keyval;
#endif

  /* Check for valid arguments */
  if ( MPIR_TEST_COMM(comm,comm) ||
	   ( (keyval == MPI_KEYVAL_INVALID) && (errno = MPI_ERR_OTHER) ) )
	return MPIR_ERROR( comm, errno, "Error in MPI_ATTR_PUT" );

  if (comm == MPI_COMM_WORLD && attr_key->permanent) 
	return MPIR_ERROR( comm, MPI_ERR_PERM_KEY, 
					  "Error in MPI_ATTR_PUT" );

  MPIR_HBT_lookup(comm->attr_cache, keyval, &attr);
  if (attr == (MPIR_HBT_node *)0) {
	(void) MPIR_HBT_new_node ( keyval, attr_value, &attr );
	(void) MPIR_HBT_insert ( comm->attr_cache, attr );
  }
  else {
	if ( attr_key->delete_fn != (int(*)())0 ) {
	    if (attr_key->FortranCalling) 
		(void) attr_key->delete_fn(comm, keyval, &attr->value,
					   attr_key->extra_state );
	    else
		(void) attr_key->delete_fn(comm, keyval, attr->value,
					   attr_key->extra_state );
	    }
	attr->value = attr_value;
  }
  return (errno);
}
