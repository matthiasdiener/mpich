/*
 *  $Id: group_incl.c,v 1.13 1994/09/30 22:11:43 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Group_incl - Produces a group by reordering an existing group and taking
        only listed members

Input Parameters:
. group - group (handle) 
. n - number of elements in array ranks (and size of newgroup ) (integer) 
. ranks - ranks of processes in group to appear in newgroup (array of 
integers) 

Output Parameter:
. newgroup - new group derived from above, in the order defined by ranks 
(handle) 

Note:
This implementation does not currently check to see that the list of
ranks to include are valid ranks in the group.

@*/
int MPI_Group_incl ( group, n, ranks, group_out )
MPI_Group group, *group_out;
int       n, *ranks;
{
  int       i, rank;
  MPI_Group new_group;
  int       errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) ||
 	   ( ((n>0)&&MPIR_TEST_ARG(ranks)) )   )
    return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_GROUP_INCL" );

  /* Check for a EMPTY input group or EMPTY sized new group */
  if ( (group == MPI_GROUP_EMPTY) || (n <= 0) ) {
    MPIR_Group_dup ( MPI_GROUP_EMPTY, group_out );
    return (errno);
  }
  
  /* Create the new group */
  new_group = (*group_out)  = NEW(struct MPIR_GROUP);
  if (!new_group) 
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_INCL" );
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count      = 1;
  new_group->local_rank     = MPI_UNDEFINED;
  new_group->permanent      = 0;
  new_group->set_mark       = (int *)0;
  new_group->np             = n;
  new_group->lrank_to_grank = (int *) MALLOC( n * sizeof(int) );
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_INCL" );
  }

  /* Fill in the lrank_to_grank list */
  for (i=0; i<n; i++) {
    if ( (rank = ranks[i]) < group->np )
      new_group->lrank_to_grank[i] = group->lrank_to_grank[rank];
    else
      new_group->lrank_to_grank[i] = MPI_UNDEFINED;
    if (group->local_rank == rank)
      new_group->local_rank = i;
  }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next), 
		     &(new_group->N2_prev) );

  return (errno);
}


