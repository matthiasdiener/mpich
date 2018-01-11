/*
 *  $Id: group_size.c,v 1.10 1994/12/15 19:30:27 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_size - Returns the size of a group

Input Parameters:
. group - group (handle) 
Output Parameter:
. size - number of processes in the group (integer) 

@*/
int MPI_Group_size ( group, size )
MPI_Group group;
int *size;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) || MPIR_TEST_ARG(size))
	return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,"Error in MPI_GROUP_SIZE");

  /* Get the size of the group */
  (*size) = group->np;

  return (mpi_errno);
}

