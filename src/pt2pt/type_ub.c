/*
 *  $Id: type_ub.c,v 1.9 1996/01/03 19:03:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_ub.c,v 1.9 1996/01/03 19:03:31 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_ub - Returns the upper bound of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. displacement - displacement of upper bound from origin, 
                             in bytes (integer) 

.N fortran
@*/
int MPI_Type_ub ( datatype, displacement )
MPI_Datatype  datatype;
MPI_Aint      *displacement;
{
  int mpi_errno;
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype) || 
      MPIR_TEST_ARG(displacement))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_TYPE_UB" );

  /* Assign the ub and return */

  MPIR_GET_REAL_DATATYPE(datatype)
  (*displacement) = datatype->ub;
  return (MPI_SUCCESS);
}
