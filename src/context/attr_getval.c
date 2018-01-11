/*
 *  $Id: attr_getval.c,v 1.13 1994/07/13 15:44:23 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: attr_getval.c,v 1.13 1994/07/13 15:44:23 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@C

MPI_Attr_get - Retrieves attribute value by key

Input Parameters:
. comm - communicator to which attribute is attached (handle) 
. keyval - key value (integer) 

Output Parameters:
. attribute_val - attribute value, unless flag = false 
. flag -  true if an attribute value was extracted;  false if no attribute is associated with the key 

@*/
int MPI_Attr_get ( comm, keyval, attr_value, flag )
MPI_Comm comm;
int keyval;
void **attr_value;
int *flag;
{
  MPIR_HBT_node *attr;
  int errno = MPI_SUCCESS;

  if ( MPIR_TEST_COMM(comm,comm)||
	   ( (keyval == MPI_KEYVAL_INVALID) && (errno = MPI_ERR_OTHER) ) )
	return MPIR_ERROR(comm, errno, "Error in MPI_ATTR_GET");
		  
  MPIR_HBT_lookup(comm->attr_cache, keyval, &attr);
  if ( attr == (MPIR_HBT_node *)0 ) {
	(*flag) = MPIR_FALSE;
	(*attr_value) = (void *)0; 
  }
  else {
	(*flag) = MPIR_TRUE;
	(*attr_value) = attr->value;
  }
  return(errno);
}



