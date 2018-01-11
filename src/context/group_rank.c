/*
 *  $Id: group_rank.c,v 1.1.1.1 1997/09/17 20:41:42 gropp Exp $
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
. rank - rank of the calling process in group, or   'MPI_UNDEFINED'  if the 
process is not a member (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_ARG

@*/
int MPI_Group_rank ( group, rank )
MPI_Group group;
int *rank;
{
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_GROUP *group_ptr;
  static char myname[] = "MPI_GROUP_RANK";

  TR_PUSH(myname);

  group_ptr = MPIR_GET_GROUP_PTR(group);
  MPIR_TEST_MPI_GROUP(group,group_ptr,MPIR_COMM_WORLD,myname);

  /* Check for invalid arguments */
  if ( MPIR_TEST_ARG(rank) )
      return MPIR_ERROR(MPIR_COMM_WORLD,mpi_errno,myname );

  /* Get the rank of the group */
  (*rank) = group_ptr->local_rank;

  TR_POP;
  return (mpi_errno);
}
