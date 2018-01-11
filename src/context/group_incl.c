/*
 *  $Id: group_incl.c,v 1.4 1998/04/28 20:58:09 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*@

MPI_Group_incl - Produces a group by reordering an existing group and taking
        only listed members

Input Parameters:
+ group - group (handle) 
. n - number of elements in array 'ranks' (and size of newgroup ) (integer) 
- ranks - ranks of processes in 'group' to appear in 'newgroup' (array of 
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
.N MPI_ERR_RANK

.seealso: MPI_Group_free
@*/
int MPI_Group_incl ( group, n, ranks, group_out )
MPI_Group group, *group_out;
int       n, *ranks;
{
  int       i, j, rank;
  struct MPIR_GROUP *group_ptr, *new_group_ptr;
  int       mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_GROUP_INCL";
  /* Check for bad arguments */

  TR_PUSH(myname);

  group_ptr = MPIR_GET_GROUP_PTR(group);
  MPIR_TEST_MPI_GROUP(group,group_ptr,MPIR_COMM_WORLD,myname);

  if ( ( ((n>0)&&MPIR_TEST_ARG(ranks)) )   )
    return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  if (n < 0) 
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );

  /* Check for a EMPTY input group or EMPTY sized new group */
  if ( (group == MPI_GROUP_EMPTY) || (n <= 0) ) {
      MPIR_Group_dup ( MPIR_GROUP_EMPTY, &new_group_ptr );
      *group_out = new_group_ptr->self;
      return (mpi_errno);
  }

  /* Check that the ranks are in range, at least (still need to check for
     duplicates) */
  for (i=0; i<n; i++) {
      if (ranks[i] < 0 || ranks[i] >= group_ptr->np) {
	  MPIR_ERROR_PUSH_ARG(&ranks[i]);
	  return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );
	  }
  /*** Check for duplicate ranks - Debbie Swider 11/18/97 ***/
      else {
          for (j=i+1; j<n; j++) {
	      if (ranks[i] == ranks[j]) {
		  MPIR_ERROR_PUSH_ARG(&ranks[j]);
		  return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );
	      }
	  }
      }
  }

  /* Create the new group */
  MPIR_ALLOC(new_group_ptr,NEW(struct MPIR_GROUP),MPIR_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "MPI_GROUP_INCL" );
  *group_out = (MPI_Group) MPIR_FromPointer( new_group_ptr );
  new_group_ptr->self = *group_out;
  MPIR_SET_COOKIE(new_group_ptr,MPIR_GROUP_COOKIE)
  new_group_ptr->ref_count      = 1;
  new_group_ptr->local_rank     = MPI_UNDEFINED;
  new_group_ptr->permanent      = 0;
  new_group_ptr->set_mark       = (int *)0;
  new_group_ptr->np             = n;
  MPIR_ALLOC(new_group_ptr->lrank_to_grank,(int *) MALLOC( n * sizeof(int) ),
	     MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, "MPI_GROUP_INCL" );

  /* Fill in the lrank_to_grank list */
  for (i=0; i<n; i++) {
      rank = ranks[i];
      /* Note ranks already checked for validity */
      new_group_ptr->lrank_to_grank[i] = group_ptr->lrank_to_grank[rank];
      if (group_ptr->local_rank == rank)
	  new_group_ptr->local_rank = i;
  }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group_ptr->np, &(new_group_ptr->N2_next), 
		     &(new_group_ptr->N2_prev) );

  TR_POP;
  return (mpi_errno);
}


