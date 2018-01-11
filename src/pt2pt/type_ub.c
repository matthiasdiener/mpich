/*
 *  $Id: type_ub.c,v 1.13 1997/01/07 01:45:29 gropp Exp $
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
  struct MPIR_DATATYPE *dtype_ptr;
  static char myname[] = "MPI_TYPE_UB";

  TR_PUSH(myname);
  if (MPIR_TEST_ARG(displacement))
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  /* Assign the ub and return */

  dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD, myname);

  (*displacement) = dtype_ptr->ub;
  TR_POP;
  return (MPI_SUCCESS);
}
