/*
 *  $Id: type_ub.c,v 1.10 1996/04/11 20:26:05 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif


/*@
    MPI_Type_ub - Returns the upper bound of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. displacement - displacement of upper bound from origin, 
                             in bytes (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
.N MPI_ERR_ARG
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
