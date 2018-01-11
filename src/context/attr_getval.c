/*
 *  $Id: attr_getval.c,v 1.19 1997/01/07 01:47:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "attr.h"
#endif

/*@C

MPI_Attr_get - Retrieves attribute value by key

Input Parameters:
. comm - communicator to which attribute is attached (handle) 
. keyval - key value (integer) 

Output Parameters:
. attr_value - attribute value, unless 'flag' = false 
. flag -  true if an attribute value was extracted;  false if no attribute is
  associated with the key 

Notes:
    Attributes must be extracted from the same language as they were inserted  
    in with 'MPI_ATTR_PUT'.  The notes for C and Fortran below explain why.

Notes for C:
    Even though the 'attr_value' arguement is declared as 'void *', it is
    really the address of a void pointer.  See the rationale in the 
    standard for more details. 

.N fortran

    The 'attr_value' in Fortran is a pointer to a Fortran integer, not
    a pointer to a 'void *'.  

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_KEYVAL
@*/
int MPI_Attr_get ( comm, keyval, attr_value, flag )
MPI_Comm comm;
int keyval;
void *attr_value;
int *flag;
{
  MPIR_HBT_node *attr;
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_ATTR_GET";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if ( ( (keyval == MPI_KEYVAL_INVALID) && (mpi_errno = MPI_ERR_OTHER) ) )
	return MPIR_ERROR(comm_ptr, mpi_errno, myname);
		  
  MPIR_HBT_lookup(comm_ptr->attr_cache, keyval, &attr);
  if ( attr == (MPIR_HBT_node *)0 ) {
	(*flag) = 0;
	(*(void **)attr_value) = (void *)0; 
  }
  else {
	(*flag) = 1;
	(*(void **)attr_value) = attr->value;
  }
  TR_POP;
  return(mpi_errno);
}



