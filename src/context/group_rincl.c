/*
 *  $Id: group_rincl.c,v 1.15 1994/12/15 19:28:57 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Group_range_incl - Creates a new group from ranges of ranks in an 
        existing group

Input Parameters:
. group - group (handle) 
. n - number of triplets in array  ranges (integer) 
. ranges - a one-dimensional array of integer triplets, of the 
form (first rank, last rank, stride) indicating ranks in
group  or processes to be included in newgroup  

Output Parameter:
. newgroup - new group derived from above, in the 
order defined by  ranges  (handle)  

Note:
This implementation does not currently check to see that the list of
ranges to include are valid ranks in the group.

@*/
int MPI_Group_range_incl ( group, n, ranges, newgroup )
MPI_Group group, *newgroup;
int       n, ranges[][3];
{
  int i, j, k, ranks, first, last, stride;
  int np = 0;
  MPI_Group new_group;
  int mpi_errno = MPI_SUCCESS;

  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) )
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
					  "Error in MPI_GROUP_RANGE_INCL" );

  /* Check for a EMPTY input group or EMPTY sized new group */
  if ( (group == MPI_GROUP_EMPTY) || (n <= 0) ) {
    MPIR_Group_dup ( MPI_GROUP_EMPTY, newgroup );
    return (mpi_errno);
  }
  
  /* Determine the number of ranks that will be included */
  for (i=0; i<n; i++)
    if (ranges[i][2] != 0)
      if ( (ranks=((ranges[i][1]-ranges[i][0])/ranges[i][2])+1) > 0 )
        np += ranks;

  /* Check for np == 0 ranks to include */
  if ( np <=0 ) {
    MPIR_Group_dup ( MPI_GROUP_EMPTY, newgroup );
    return (mpi_errno);
  }

  /* Create the new group */
  new_group = (*newgroup)   = NEW(struct MPIR_GROUP);
  if (!new_group) 
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
				  "Out of space in MPI_GROUP_RANGE_INCL" );
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count      = 1;
  new_group->permanent      = 0;
  new_group->local_rank     = MPI_UNDEFINED;
  new_group->set_mark       = (int *)0;
  new_group->np             = np;
  new_group->lrank_to_grank = (int *) MALLOC( np * sizeof(int) );
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
				  "Out of space in MPI_GROUP_RANGE_INCL" );
  }

  /* Fill in the lrank_to_grank list */
  k = 0;
  for (i=0; i<n; i++) {
    first = ranges[i][0]; last = ranges[i][1]; stride = ranges[i][2];
    if (stride != 0)
      for ( j=first; j*stride <= last*stride; j += stride ) {
        if ( (j < group->np) && (j >= 0) ) {
          if ( group->local_rank == j )  
            new_group->local_rank = k;
          new_group->lrank_to_grank[k++] = group->lrank_to_grank[j];
        }
        else
          new_group->lrank_to_grank[k++] = MPI_UNDEFINED;
      }
  }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next), 
		     &(new_group->N2_prev) );

  return (mpi_errno);
}

