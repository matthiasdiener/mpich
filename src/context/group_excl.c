/*
 *  $Id: group_excl.c,v 1.13 1994/09/30 22:11:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Group_excl - Produces a group by reordering an existing group and taking
        only unlisted members

Input Parameters:
. group - group (handle) 
. n - number of elements in array ranks (integer) 
. ranks - array of integer ranks in group not to appear in newgroup 

Output Parameter:
. newgroup - new group derived from above, preserving the order defined by  group (handle) 

Note:  
Currently, each of the ranks to exclude must be
a valid rank in the group and all elements must be distinct or the
function is erroneous.  This restriction is per the draft.
@*/
int MPI_Group_excl ( group, n, ranks, newgroup )
MPI_Group group, *newgroup;
int       n, *ranks;
{
  int i, j, rank;
  MPI_Group new_group;
  int errno = MPI_SUCCESS;

  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) || 
      (n > 0 && MPIR_TEST_ARG(ranks)))
    return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_GROUP_EXCL" );

  /* Check for a EMPTY input group */
  if ( (group == MPI_GROUP_EMPTY) || (n >= group->np) ) {
	MPIR_Group_dup ( MPI_GROUP_EMPTY, newgroup );
    return (errno);
  }
  
  /* Check for no ranks to exclude */
  if ( n <= 0 ) {
    (void) MPIR_Group_dup ( group, newgroup );
    return (errno);
  }

  /* Create the new group */
  new_group	= (*newgroup)   = NEW(struct MPIR_GROUP);
  if(!new_group)
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_EXCL" );
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count	    = 1;
  new_group->local_rank	    = MPI_UNDEFINED;
  new_group->permanent      = 0;
  new_group->set_mark	    = (int *)0;
  new_group->np             = group->np - n;
  new_group->lrank_to_grank = (int *)MALLOC(new_group->np * sizeof(int));
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_EXCL" );
  }

  /* Allocate set marking space for group if necessary */
  if (group->set_mark == NULL) {
    group->set_mark = (int *) MALLOC( group->np * sizeof(int) );
    if (!group->set_mark)
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
						"Out of space in MPI_GROUP_EXCL" );
    }
  (void) memset( group->set_mark, (char)0, group->np * sizeof(int) );

  /* Mark the ranks to be excluded */
  for (i=0; i<n; i++) 
    if ( ((rank = ranks[i]) < group->np) && (rank >= 0) ) 
      group->set_mark[rank] = MPIR_MARKED;

  /* Fill in the lrank_to_grank list */
  for (i=j=0; i < group->np ; i++) 
    if ( (group->set_mark[i] == MPIR_UNMARKED) && (j < new_group->np ) ) {
      if (group->local_rank == i)
        new_group->local_rank = j;
      new_group->lrank_to_grank[j++] = group->lrank_to_grank[i];
    }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next), &(new_group->N2_prev) );

  return (errno);
}
