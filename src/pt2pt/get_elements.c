/*
 *  $Id: get_elements.c,v 1.6 1994/10/24 22:02:45 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: get_elements.c,v 1.6 1994/10/24 22:02:45 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
  MPI_Get_elements - Returns the number of basic elements
                     in a datatype

Input Parameters:
. status - return status of receive operation (Status) 
. datatype - datatype used by receive operation (handle) 

Output Parameter:
. count - number of received basic elements (integer) 
@*/
int MPI_Get_elements ( status, datatype, elements )
MPI_Status    *status;
MPI_Datatype  datatype;
int          *elements;
{
  int count, errno;

  if (MPIR_TEST_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, 
			   "Error in MPI_GET_ELEMENTS" );
  
  /* Find the number of elements */
  MPI_Get_count (status, datatype, &count);
  if (count == MPI_UNDEFINED) {
	/* HACK ALERT -- the code in this if is not correct */
	/*               but for now ... */
	double cnt = 
	  (double) status->count / (double) datatype->size;
	(*elements) = (int) ( cnt * (double) datatype->elements );
  }
  else
	(*elements) = count * datatype->elements;

  return (MPI_SUCCESS);
}

