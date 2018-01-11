/*
 *  $Id: type_size.c,v 1.12 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif


/*@
    MPI_Type_size - Return the number of bytes occupied by entries
                    in the datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. size - datatype size (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
.N MPI_ERR_ARG
@*/
int MPI_Type_size ( datatype, size )
MPI_Datatype  datatype;
int           *size;
{
  int mpi_errno;
  struct MPIR_DATATYPE *dtype_ptr;
  static char myname[] = "MPI_TYPE_SIZE";

  TR_PUSH(myname);
  if (MPIR_TEST_ARG(size))
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

  /* Assign the size and return */
  (*size) = (int)(dtype_ptr->size);
  TR_POP;
  return (MPI_SUCCESS);
}
