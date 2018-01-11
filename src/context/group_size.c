/*
 *  $Id: group_size.c,v 1.9 1994/07/13 15:47:52 lusk Exp $
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
  int errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) || MPIR_TEST_ARG(size))
	return MPIR_ERROR(MPI_COMM_WORLD,errno,"Error in MPI_GROUP_SIZE");

  /* Get the size of the group */
  (*size) = group->np;

  return (errno);
}

