/*
 *  $Id: group_excl.c,v 1.21 1997/01/17 22:59:30 gropp Exp $
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

MPI_Group_excl - Produces a group by reordering an existing group and taking
        only unlisted members

Input Parameters:
. group - group (handle) 
. n - number of elements in array 'ranks' (integer) 
. ranks - array of integer ranks in 'group' not to appear in 'newgroup' 

Output Parameter:
. newgroup - new group derived from above, preserving the order defined by 
 'group' (handle) 

Note:  
Currently, each of the ranks to exclude must be
a valid rank in the group and all elements must be distinct or the
function is erroneous.  This restriction is per the draft.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_EXHAUSTED
.N MPI_ERR_ARG
.N MPI_ERR_RANK

.seealso: MPI_Group_free
@*/
int MPI_Group_excl ( group, n, ranks, newgroup )
MPI_Group group, *newgroup;
int       n, *ranks;
{
  int i, j, rank;
  struct MPIR_GROUP *new_group_ptr;
  struct MPIR_GROUP *group_ptr;
  int mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_GROUP_EXCL";

  TR_PUSH(myname);

  group_ptr = MPIR_GET_GROUP_PTR(group);
  MPIR_TEST_MPI_GROUP(group,group_ptr,MPIR_COMM_WORLD,myname);

  if (n < 0) 
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );

  if ( (n > 0 && MPIR_TEST_ARG(ranks)))
    return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  /* Error if we're excluding more than the size of the group */
  if ( n > group_ptr->np ) {
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );
  }

  /* Check for a EMPTY input group */
  if ( (group == MPI_GROUP_EMPTY) || (n == group_ptr->np) ) {
      MPIR_Group_dup ( MPIR_GROUP_EMPTY, &new_group_ptr );
      *newgroup = new_group_ptr->self;
      return MPI_SUCCESS;
  }
  
  /* Check for no ranks to exclude */
  if ( n <= 0 ) {
    MPIR_Group_dup ( group_ptr, &new_group_ptr );
    *newgroup = new_group_ptr->self;
    return (mpi_errno);
  }

  /* Create the new group */
  MPIR_ALLOC(new_group_ptr,NEW(struct MPIR_GROUP),MPIR_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "Out of space in MPI_GROUP_EXCL" );
  new_group_ptr->self = MPIR_FromPointer( new_group_ptr );
  *newgroup = new_group_ptr->self;
  MPIR_SET_COOKIE(new_group_ptr,MPIR_GROUP_COOKIE)
  new_group_ptr->ref_count	    = 1;
  new_group_ptr->local_rank	    = MPI_UNDEFINED;
  new_group_ptr->permanent      = 0;
  new_group_ptr->set_mark	    = (int *)0;
  new_group_ptr->np             = group_ptr->np - n;
  MPIR_ALLOC(new_group_ptr->lrank_to_grank,
	     (int *)MALLOC(new_group_ptr->np * sizeof(int)),
	     MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_GROUP_EXCL" );

  /* Allocate set marking space for group if necessary */
  if (group_ptr->set_mark == NULL) {
      MPIR_ALLOC(group_ptr->set_mark,(int *) MALLOC( group_ptr->np * sizeof(int) ),
		 MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		 "Out of space in MPI_GROUP_EXCL" );
  }
  (void) memset( group_ptr->set_mark, (char)0, group_ptr->np * sizeof(int) );

  /* Mark the ranks to be excluded */
  for (i=0; i<n; i++) {
    if ( ((rank = ranks[i]) < group_ptr->np) && (rank >= 0) ) 
	if (group_ptr->set_mark[rank] == MPIR_MARKED) {
	    /* Do not allow duplicate ranks */
	    MPIR_ERROR_PUSH_ARG(&rank);
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );
	}
	else
	    group_ptr->set_mark[rank] = MPIR_MARKED;
    else {
	/* Out of range rank in input */
	MPIR_ERROR_PUSH_ARG(&rank);
        return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );
    }
  }

  /* Fill in the lrank_to_grank list */
  for (i=j=0; i < group_ptr->np ; i++) 
    if ( (group_ptr->set_mark[i] == MPIR_UNMARKED) && (j < new_group_ptr->np ) ) {
      if (group_ptr->local_rank == i)
        new_group_ptr->local_rank = j;
      new_group_ptr->lrank_to_grank[j++] = group_ptr->lrank_to_grank[i];
    }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group_ptr->np, &(new_group_ptr->N2_next), &(new_group_ptr->N2_prev) );

  TR_POP;

  return MPI_SUCCESS;
}
