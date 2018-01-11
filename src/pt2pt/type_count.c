/*
 *  $Id: type_count.c,v 1.5 1994/09/21 15:27:12 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_count.c,v 1.5 1994/09/21 15:27:12 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_count - Returns the number of "top-level" entries in 
	                 the datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. count - datatype count (integer) 
@*/
int MPI_Type_count ( datatype, count )
MPI_Datatype  datatype;
int          *count;
{
  int errno;
  /* Check for bad datatype */
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_TYPE_COUNT" );

  /* Assign the count and return */
  (*count) = datatype->count;
  return (MPI_SUCCESS);
}

