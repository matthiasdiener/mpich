/*
 *  $Id: type_size.c,v 1.9 1996/04/11 20:25:46 gropp Exp $
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
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype) || MPIR_TEST_ARG(size))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_TYPE_SIZE" );

  /* Assign the size and return */
  /* For SOME versions, could use just MPIR_DATATYPE_SIZE */
  MPIR_GET_REAL_DATATYPE(datatype)
  (*size) = (int)(datatype->size);
  return (MPI_SUCCESS);
}
