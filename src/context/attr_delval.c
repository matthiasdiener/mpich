/*
 *  $Id: attr_delval.c,v 1.20 1996/05/06 15:05:44 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "attr.h"
#endif

/*@

MPI_Attr_delete - Deletes attribute value associated with a key

Input Parameters:
. comm - communicator to which attribute is attached (handle) 
. keyval - The key value of the deleted attribute (integer) 

.N fortran

.N Errors
.N MPI_ERR_COMM
.N MPI_ERR_PERM_KEY
@*/
int MPI_Attr_delete ( comm, keyval )
MPI_Comm comm;
int      keyval;
{
  MPIR_HBT_node *attr;
  MPIR_Attr_key *attr_key;
  int            mpi_errno    = MPI_SUCCESS;

#ifdef INT_LT_POINTER
  attr_key = (MPIR_Attr_key *)MPIR_ToPointer( keyval );
#else
  attr_key = (MPIR_Attr_key *)keyval;
#endif
  
  if ( MPIR_TEST_COMM(comm,comm) ||
	   ( (keyval == MPI_KEYVAL_INVALID) && (mpi_errno = MPI_ERR_OTHER) ) )
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ATTR_DELETE");

  if (comm == MPI_COMM_WORLD && attr_key->permanent) 
	return MPIR_ERROR( comm, MPI_ERR_PERM_KEY, 
					  "Error in MPI_ATTR_DELETE" );

  MPIR_HBT_lookup(comm->attr_cache, keyval, &attr);
  if (attr != (MPIR_HBT_node *)0) {
	if ( attr_key->delete_fn.c_delete_fn ) {
	    if (attr_key->FortranCalling) {
		MPI_Aint  invall = (MPI_Aint)attr->value;
		int inval = (int)invall;
		mpi_errno = (*attr_key->delete_fn.f77_delete_fn)(comm, 
					    &keyval, &inval,
					    attr_key->extra_state );
		attr->value = (void *)(MPI_Aint)inval;
	    }
	    else
		mpi_errno = (*attr_key->delete_fn.c_delete_fn)(comm, 
					    keyval, attr->value,
					    attr_key->extra_state );
	    }
	MPIR_HBT_delete(comm->attr_cache, keyval, &attr);
	/* We will now have one less reference to keyval */
	attr_key->ref_count--;
	if ( attr != (MPIR_HBT_node *)0 ) 
	  (void) MPIR_HBT_free_node ( attr );
	}
  else 
      return MPIR_ERROR( comm, MPI_ERR_OTHER, 
		  "Error in MPI_ATTR_DELETE: key not in communicator" );

  return(mpi_errno);
}
