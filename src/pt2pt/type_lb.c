/*
 *  $Id: type_lb.c,v 1.13 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif


/*@
    MPI_Type_lb - Returns the lower-bound of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. displacement - displacement of lower bound from origin, 
                             in bytes (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
.N MPI_ERR_ARG
@*/
int MPI_Type_lb ( datatype, displacement )
MPI_Datatype  datatype;
MPI_Aint      *displacement;
{
  int mpi_errno;
  struct MPIR_DATATYPE *dtype_ptr;
  static char myname[] = "MPI_TYPE_LB";

  TR_PUSH(myname);

  dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

  if (MPIR_TEST_ARG(displacement))
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname);

  /* Assign the lb and return */
  (*displacement) = dtype_ptr->lb;

  TR_POP;
  return (MPI_SUCCESS);
}
