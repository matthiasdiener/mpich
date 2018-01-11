/*
 *  $Id: type_size.c,v 1.7 1995/07/26 16:55:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_size.c,v 1.7 1995/07/26 16:55:07 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_size - Return the number of bytes occupied by entries
                    in the datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. size - datatype size (integer) 
@*/
int MPI_Type_size ( datatype, size )
MPI_Datatype  datatype;
MPI_Aint      *size;
{
  int mpi_errno;
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype) || MPIR_TEST_ARG(size))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_TYPE_SIZE" );

  /* Assign the size and return */
  (*size) = datatype->size;
  return (MPI_SUCCESS);
}
