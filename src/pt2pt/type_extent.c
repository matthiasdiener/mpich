/*
 *  $Id: type_extent.c,v 1.5 1994/09/21 15:27:13 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_extent.c,v 1.5 1994/09/21 15:27:13 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_extent - Returns the extent of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. extent - datatype extent (integer) 
@*/
int MPI_Type_extent( datatype, extent )
MPI_Datatype  datatype;
MPI_Aint     *extent;
{
  int errno;
  /* Check for bad datatype */
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_TYPE_EXTENT" );

  /* Assign the extent and return */
  (*extent) = datatype->extent;
  return (MPI_SUCCESS);
}
