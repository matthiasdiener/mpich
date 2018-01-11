/*
 *  $Id: type_count.c,v 1.1.1.1 1997/09/17 20:42:10 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_count.c,v 1.1.1.1 1997/09/17 20:42:10 gropp Exp $";
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
  static char myname[] = "MPI_TYPE_COUNT";

  TR_PUSH(myname);
  /* Check for bad datatype */
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, myname );

  /* Assign the count and return */
  (*count) = datatype->count;
  TR_POP;
  return (MPI_SUCCESS);
}

