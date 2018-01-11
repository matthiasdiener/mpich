/*
 *  $Id: type_lb.c,v 1.6 1994/09/21 15:27:15 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_lb.c,v 1.6 1994/09/21 15:27:15 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_lb - Returns the lower-bound of a datatype

Input Parameters:
. datatype - datatype (handle) 

Output Parameter:
. displacement - displacement of lower bound from origin, 
                             in bytes (integer) 
@*/
int MPI_Type_lb ( datatype, displacement )
MPI_Datatype  datatype;
MPI_Aint      *displacement;
{
  int errno;

  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,datatype) || 
      MPIR_TEST_ARG(displacement))
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_TYPE_LB" );

  /* Assign the lb and return */
  (*displacement) = datatype->lb;
  return (MPI_SUCCESS);
}
