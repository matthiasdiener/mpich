/*
 *  $Id: type_vec.c,v 1.13 1996/07/17 18:04:00 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Type_vector - Creates a vector (strided) datatype

Input Parameters:
. count - number of blocks (nonnegative integer) 
. blocklength - number of elements in each block 
(nonnegative integer) 
. stride - number of elements between start of each block (integer) 
. oldtype - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 

.N fortran
@*/
int MPI_Type_vector( count, blocklen, stride, old_type, newtype )
int          count;
int 	     blocklen;
int 	     stride;
MPI_Datatype old_type;
MPI_Datatype *newtype;
{
  int           mpi_errno = MPI_SUCCESS;
  MPIR_ERROR_DECL;

  /* Check for bad arguments */
  MPIR_GET_REAL_DATATYPE(old_type)
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (count   <  0)                  && (mpi_errno = MPI_ERR_COUNT) ) ||
   ( (blocklen <  0)                 && (mpi_errno = MPI_ERR_ARG) )   ||
   ( (old_type->dte_type == MPIR_UB) && (mpi_errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (mpi_errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_TYPE_VECTOR" );
	
  /* Handle the case where blocklen & stride make a contiguous type */
  MPIR_ERROR_PUSH(MPI_COMM_WORLD);

  if ( (blocklen == stride) || (count    == 1) ) {
	MPIR_CALL_POP(MPI_Type_contiguous ( 
	    count * blocklen, old_type, newtype ),MPI_COMM_WORLD,
		      "Error in MPI_TYPE_VECTOR");
  }
  /* Reduce this to the hvector case */
  
  mpi_errno = MPI_Type_hvector ( count, blocklen, 
				 (MPI_Aint)stride * old_type->extent,
						    old_type, newtype );
  MPIR_ERROR_POP(MPI_COMM_WORLD);
  MPIR_RETURN(MPI_COMM_WORLD,mpi_errno,"Error in MPI_TYPE_VECTOR");
}
