/*
 *  $Id: getcount.c,v 1.7 1994/10/24 22:02:45 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: getcount.c,v 1.7 1994/10/24 22:02:45 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
  MPI_Get_count - Gets the number of "top level" elements

Input Parameters:
. status - return status of receive operation (Status) 
. datatype - datatype of each receive buffer element (handle) 

Output Parameter:
. count - number of received elements (integer) 
  
@*/
int MPI_Get_count( status, datatype, count )
MPI_Status   *status;
MPI_Datatype datatype;
int          *count;
{
  int errno;
  if (MPIR_TEST_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_GET_COUNT" );

  /* Check for correct number of bytes */
  if ((status->count % (datatype->size)) != 0)
	(*count) = MPI_UNDEFINED;
  else
	(*count) = status->count / (datatype->size);

  return (MPI_SUCCESS);
}
