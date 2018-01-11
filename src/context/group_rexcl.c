/*
 *  $Id: group_rexcl.c,v 1.14 1994/09/30 22:11:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"
/*@

MPI_Group_range_excl - Produces a group by excluding ranges of processes from
       an existing group

Input Parameters:
. group - group (handle) 
. n - number of elements in array ranks (integer) 
. ranges - a one-dimensional 
array of integer triplets of the
form (first rank, last rank, stride), indicating the ranks in
group  of processes to be excluded
from the output group newgroup .  

Output Parameter:
. newgroup - new group derived from above, preserving the 
order in group  (handle) 

Note:  
Currently, each of the ranks to exclude must be
a valid rank in the group and all elements must be distinct or the
function is erroneous.  This restriction is per the draft.
@*/
int MPI_Group_range_excl ( group, n, ranges, newgroup )
MPI_Group group, *newgroup;
int       n, ranges[][3];
{
  int i, j, first, last, stride;
  int np;
  MPI_Group new_group;
  int errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) ) 
    return MPIR_ERROR( MPI_COMM_WORLD, errno, 
					  "Error in MPI_GROUP_RANGE_EXCL" );

  /* Check for a EMPTY input group */
  if ( (group == MPI_GROUP_EMPTY) ) {
    MPIR_Group_dup ( MPI_GROUP_EMPTY, newgroup );
    return (errno);
  }

  /* Check for no range ranks to exclude */
  if ( n <= 0 ) {
    (void) MPIR_Group_dup ( group, newgroup );
    return (errno);
  }

  /* Allocate set marking space for group if necessary */
  if (group->set_mark == NULL) {
    group->set_mark = (int *) MALLOC( group->np * sizeof(int) );
    if (!group->set_mark) 
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
						"Out of space in MPI_GROUP_RANGE_EXCL" );
  }
  (void) memset( group->set_mark, (char)0, group->np * sizeof(int) );

  /* Mark the ranks to be excluded */
  np = group->np;  
  for (i=0; i<n; i++) {
    first = ranges[i][0]; last = ranges[i][1]; stride = ranges[i][2];
    if (stride != 0)
      for ( j=first; j*stride <= last*stride; j += stride )
        if ( (j < group->np) && (j >= 0) ) 
          if (group->set_mark[j] == MPIR_UNMARKED) {
            group->set_mark[j] = MPIR_MARKED;
            np--;
          }
  }

  /* Check np to see if we have original group or if we have null group */
  if (np == 0) {
    MPIR_Group_dup ( MPI_GROUP_EMPTY, newgroup );
    return (errno);
  }
  if (np == group->np) {
    (void) MPIR_Group_dup ( group, newgroup );
    return (errno);
  }

  /* Create the new group */
  new_group = (*newgroup)   = NEW(struct MPIR_GROUP);
  if (!new_group) 
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_REXCL" );
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count      = 1;
  new_group->permanent      = 0;
  new_group->local_rank     = MPI_UNDEFINED;
  new_group->set_mark       = (int *)0;
  new_group->np             = np;
  new_group->lrank_to_grank = (int *) MALLOC( np * sizeof(int) );
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_RANGE_EXCL" );
  }
  
  /* Fill in new group */
  for (i=j=0; i < group->np ; i++) 
    if ( (group->set_mark[i] == MPIR_UNMARKED) && (j < new_group->np ) ) {
      if (group->local_rank == i)
        new_group->local_rank = j;
      new_group->lrank_to_grank[j++] = group->lrank_to_grank[i];
    }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next), 
		     &(new_group->N2_prev) );

  return (errno);
}

