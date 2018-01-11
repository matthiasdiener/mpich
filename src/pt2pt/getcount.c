/*
 *  $Id: getcount.c,v 1.12 1996/04/11 20:18:35 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Get_count - Gets the number of "top level" elements

Input Parameters:
. status - return status of receive operation (Status) 
. datatype - datatype of each receive buffer element (handle) 

Output Parameter:
. count - number of received elements (integer) 
  
Notes:
If the size of the datatype is zero, this routine will return a count of
zero.  If the amount of data in 'status' is not an exact multiple of the 
size of 'datatype' (so that 'count' would not be integral), a 'count' of
'MPI_UNDEFINED' is returned instead.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
@*/
int MPI_Get_count( status, datatype, count )
MPI_Status   *status;
MPI_Datatype datatype;
int          *count;
{
  int mpi_errno;
  if (MPIR_TEST_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_GET_COUNT" );

  MPIR_GET_REAL_DATATYPE(datatype)

  /* Check for correct number of bytes */
  if (datatype->size == 0) {
      if (status->count > 0)
	  (*count) = MPI_UNDEFINED;
      else
	  /* This is ambiguous */
	  (*count) = 0;
      }
  else {
      if ((status->count % (datatype->size)) != 0)
	  (*count) = MPI_UNDEFINED;
      else
	  (*count) = status->count / (datatype->size);
      }

  return (MPI_SUCCESS);
}
