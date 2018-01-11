/*
 *  $Id: type_extent.c,v 1.3 1998/06/22 14:28:23 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*@
    MPI_Type_extent - Returns the extent of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. extent - datatype extent (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
@*/
int MPI_Type_extent( datatype, extent )
MPI_Datatype  datatype;
MPI_Aint     *extent;
{
  struct MPIR_DATATYPE *dtype_ptr;
  static char myname[] = "MPI_TYPE_EXTENT";

  dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

  /* Assign the extent and return */
  (*extent) = dtype_ptr->extent;
  return (MPI_SUCCESS);
}







