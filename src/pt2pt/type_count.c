/*
 *  $Id: type_count.c,v 1.7 1995/05/09 19:00:28 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_count.c,v 1.7 1995/05/09 19:00:28 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_count - Returns the number of "top-level" entries in 
	                 the datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. count - datatype count (integer) 

   Note:
   This routine has been removed from MPI.
@*/
int MPI_Type_count ( datatype, count )
MPI_Datatype  datatype;
int          *count;
{
  int mpi_errno;
  /* Check for bad datatype */
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			  "Error in MPI_TYPE_COUNT" );

  /* Assign the count and return */
  (*count) = datatype->count;
  return (MPI_SUCCESS);
}

