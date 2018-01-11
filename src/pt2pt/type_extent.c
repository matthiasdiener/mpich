/*
 *  $Id: type_extent.c,v 1.7 1995/12/21 21:36:05 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_extent.c,v 1.7 1995/12/21 21:36:05 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_extent - Returns the extent of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. extent - datatype extent (integer) 

.N fortran
@*/
int MPI_Type_extent( datatype, extent )
MPI_Datatype  datatype;
MPI_Aint     *extent;
{
  int mpi_errno;
  /* Check for bad datatype */
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			  "Error in MPI_TYPE_EXTENT" );

  MPIR_GET_REAL_DATATYPE(datatype)
  /* Assign the extent and return */
  (*extent) = datatype->extent;
  return (MPI_SUCCESS);
}
