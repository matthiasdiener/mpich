/*
 *  $Id: getcount.c,v 1.15 1997/01/07 01:45:29 gropp Exp $
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
  struct MPIR_DATATYPE *dtype_ptr;
  static char myname[] = "MPI_GET_COUNT";

  TR_PUSH(myname);

  dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

  /* Check for correct number of bytes */
  if (dtype_ptr->size == 0) {
      if (status->count > 0)
	  (*count) = MPI_UNDEFINED;
      else
	  /* This is ambiguous */
	  (*count) = 0;
      }
  else {
      if ((status->count % (dtype_ptr->size)) != 0)
	  (*count) = MPI_UNDEFINED;
      else
	  (*count) = status->count / (dtype_ptr->size);
      }

  TR_POP;
  return (MPI_SUCCESS);
}


