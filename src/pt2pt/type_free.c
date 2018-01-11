/*
 *  $Id: type_free.c,v 1.16 1996/04/11 20:25:05 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
/* pt2pt for MPIR_Type_free */
#include "mpipt2pt.h"
#endif

#ifndef MPIR_TRUE
#define MPIR_TRUE  1
#define MPIR_FALSE 0
#endif

/*@
    MPI_Type_free - Frees the datatype

Input Parameter:
. datatype - datatype that is freed (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
.N MPI_ERR_ARG
@*/
int MPI_Type_free ( datatype )
MPI_Datatype *datatype;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if (MPIR_TEST_ARG(datatype) || 
      MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,*datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_TYPE_FREE" );

  /* Freeing null datatypes succeeds silently */
  if ( (*datatype) == MPI_DATATYPE_NULL )
	return (MPI_SUCCESS);

  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*datatype)->permanent ) && MPIR_Has_been_initialized == 1) 
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_PERM_TYPE,
					  "Error in MPI_TYPE_FREE" );

  mpi_errno = MPIR_Type_free( datatype );

  (*datatype) = MPI_DATATYPE_NULL;
  return (mpi_errno);
}

