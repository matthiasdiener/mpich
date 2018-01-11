/*
 *  $Id: attr_delval.c,v 1.3 1998/04/28 20:57:50 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "attr.h"

/*@

MPI_Attr_delete - Deletes attribute value associated with a key

Input Parameters:
+ comm - communicator to which attribute is attached (handle) 
- keyval - The key value of the deleted attribute (integer) 

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
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_ATTR_DELETE";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);
  
  if ( ( (keyval == MPI_KEYVAL_INVALID) && (mpi_errno = MPI_ERR_OTHER) ) )
	return MPIR_ERROR(comm_ptr, mpi_errno, myname);

  attr_key = MPIR_GET_KEYVAL_PTR( keyval );
  MPIR_TEST_MPI_KEYVAL(keyval,attr_key,comm_ptr,myname);

  if (comm == MPI_COMM_WORLD && attr_key->permanent) 
	return MPIR_ERROR( comm_ptr, MPI_ERR_PERM_KEY,myname );

  MPIR_HBT_lookup(comm_ptr->attr_cache, keyval, &attr);
  if (attr != (MPIR_HBT_node *)0) {
	if ( attr_key->delete_fn.c_delete_fn ) {
	    if (attr_key->FortranCalling) {
		MPI_Aint  invall = (MPI_Aint)attr->value;
		int inval = (int)invall;
		(*attr_key->delete_fn.f77_delete_fn)(comm, 
					   &keyval, &inval,
					   attr_key->extra_state, &mpi_errno );
		attr->value = (void *)(MPI_Aint)inval;
	    }
	    else
		mpi_errno = (*attr_key->delete_fn.c_delete_fn)(comm, 
					    keyval, attr->value,
					    attr_key->extra_state );
	    if (mpi_errno) 
		return MPIR_ERROR( comm_ptr, mpi_errno, myname );
	    }
	MPIR_HBT_delete(comm_ptr->attr_cache, keyval, &attr);
	/* We will now have one less reference to keyval */
	MPIR_REF_DECR(attr_key);
	if ( attr != (MPIR_HBT_node *)0 ) 
	  (void) MPIR_HBT_free_node ( attr );
	}
  else 
      return MPIR_ERROR( comm_ptr, MPI_ERR_OTHER, 
		  "Error in MPI_ATTR_DELETE: key not in communicator" );

  TR_POP;
  return(mpi_errno);
}

