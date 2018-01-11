/*
 *  $Id: group_incl.c,v 1.16 1996/04/12 14:09:53 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

/*@

MPI_Group_incl - Produces a group by reordering an existing group and taking
        only listed members

Input Parameters:
. group - group (handle) 
. n - number of elements in array 'ranks' (and size of newgroup ) (integer) 
. ranks - ranks of processes in 'group' to appear in 'newgroup' (array of 
integers) 

Output Parameter:
. newgroup - new group derived from above, in the order defined by 'ranks' 
(handle) 

Note:
This implementation does not currently check to see that the list of
ranks to ensure that there are no duplicates.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_ARG
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Group_free
@*/
int MPI_Group_incl ( group, n, ranks, group_out )
MPI_Group group, *group_out;
int       n, *ranks;
{
  int       i, rank;
  MPI_Group new_group;
  int       mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group) ||
 	   ( ((n>0)&&MPIR_TEST_ARG(ranks)) )   )
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_GROUP_INCL" );

  /* Check for a EMPTY input group or EMPTY sized new group */
  if ( (group == MPI_GROUP_EMPTY) || (n <= 0) ) {
      (void) MPIR_Group_dup ( MPI_GROUP_EMPTY, group_out );
    return (mpi_errno);
  }

  /* Check that the ranks are in range, at least (still need to check for
     duplicates) */
  for (i=0; i<n; i++) {
      if (ranks[i] < 0) {
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ARG, 
			    "Invalid rank (< 0) in MPI_GROUP_INCL" );
	  }
      if (ranks[i] >= group->np) {
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ARG, 
		    "Invalid rank (>= size of group) in MPI_GROUP_INCL" );
	  }
      }

  /* Create the new group */
  MPIR_ALLOC(new_group,NEW(struct MPIR_GROUP),MPI_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "Out of space in MPI_GROUP_INCL" );
  *group_out = new_group;
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count      = 1;
  new_group->local_rank     = MPI_UNDEFINED;
  new_group->permanent      = 0;
  new_group->set_mark       = (int *)0;
  new_group->np             = n;
  MPIR_ALLOC(new_group->lrank_to_grank,(int *) MALLOC( n * sizeof(int) ),
	     MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_GROUP_INCL" );

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

  return (mpi_errno);
}


