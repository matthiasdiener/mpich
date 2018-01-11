/*
 *  $Id: group_rank.c,v 1.10 1994/12/15 16:33:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_rank - Returns the rank of this process in the given group

Input Parameters:
. group - group (handle) 

Output Parameter:
. rank - rank of the calling process in group, or   MPI_UNDEFINED  if the 
process is not a member (integer) 

@*/
int MPI_Group_rank ( group, rank )
MPI_Group group;
int *rank;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) ||
       MPIR_TEST_ARG(rank) )
	return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,"Error in MPI_GROUP_RANK");

  /* Get the rank of the group */
  (*rank) = group->local_rank;

  return (mpi_errno);
}
