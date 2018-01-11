/*
 *  $Id: group_rincl.c,v 1.3 1998/04/28 20:58:14 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*@

MPI_Group_range_incl - Creates a new group from ranges of ranks in an 
        existing group

Input Parameters:
+ group - group (handle) 
. n - number of triplets in array  'ranges' (integer) 
- ranges - a one-dimensional array of integer triplets, of the 
form (first rank, last rank, stride) indicating ranks in
'group'  or processes to be included in 'newgroup'  

Output Parameter:
. newgroup - new group derived from above, in the 
order defined by  'ranges' (handle)  

Note:
This implementation does not currently check to see that the list of
ranges to include are valid ranks in the group.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_EXHAUSTED
.N MPI_ERR_ARG
.N MPI_ERR_RANK

.seealso: MPI_Group_free
@*/
int MPI_Group_range_incl ( group, n, ranges, newgroup )
MPI_Group group, *newgroup;
int       n, ranges[][3];
{
  int i, j, k, ranks, first, last, stride;
  int np = 0;
  struct MPIR_GROUP *group_ptr, *new_group_ptr;
  int mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_GROUP_RANGE_INCL";

  TR_PUSH(myname);

  group_ptr = MPIR_GET_GROUP_PTR(group);
  MPIR_TEST_MPI_GROUP(group,group_ptr,MPIR_COMM_WORLD,myname);

  /* Check for a EMPTY input group or EMPTY sized new group */
  if ( (group == MPI_GROUP_EMPTY) || (n == 0) ) {
      MPIR_Group_dup ( MPIR_GROUP_EMPTY, &new_group_ptr );
      *newgroup = new_group_ptr->self;
      TR_POP;
      return (mpi_errno);
  }
  
  if (n < 0) 
      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );

  /* Determine the number of ranks that will be included */
  for (i=0; i<n; i++) {
      first = ranges[i][0]; last = ranges[i][1]; stride = ranges[i][2];
      if (stride != 0) {
	  if ( (stride > 0 && first > last) ||
	       (stride < 0 && first < last) ) {
	      return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, 
				 "range non terminating" );
	  }
	  if ( (ranks=((last-first)/stride)+1) > 0 )
	      np += ranks;
      }
      else {
	  /* Zero stride */
	  return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ARG, myname );
      }
  }
  /* Check for np == 0 ranks to include */
  if ( np <=0 ) {
      MPIR_Group_dup ( MPIR_GROUP_EMPTY, &new_group_ptr );
      *newgroup = new_group_ptr->self;
      return (mpi_errno);
  }

  /* Create the new group */
  MPIR_ALLOC(new_group_ptr,NEW(struct MPIR_GROUP),MPIR_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "MPI_GROUP_RANGE_INCL" );
  *newgroup = (MPI_Group) MPIR_FromPointer( new_group_ptr ); 
  new_group_ptr->self = *newgroup;
  MPIR_SET_COOKIE(new_group_ptr,MPIR_GROUP_COOKIE)
  new_group_ptr->ref_count      = 1;
  new_group_ptr->permanent      = 0;
  new_group_ptr->local_rank     = MPI_UNDEFINED;
  new_group_ptr->set_mark       = (int *)0;
  new_group_ptr->np             = np;
  MPIR_ALLOC(new_group_ptr->lrank_to_grank,(int *) MALLOC( np * sizeof(int) ),
	     MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
	     "MPI_GROUP_RANGE_INCL" );

  /* Fill in the lrank_to_grank list */
  k = 0;
  for (i=0; i<n; i++) {
    first = ranges[i][0]; last = ranges[i][1]; stride = ranges[i][2];
    if (stride != 0)
      for ( j=first; j*stride <= last*stride; j += stride ) {
        if ( (j < group_ptr->np) && (j >= 0) ) {
          if ( group_ptr->local_rank == j )  
            new_group_ptr->local_rank = k;
          new_group_ptr->lrank_to_grank[k++] = group_ptr->lrank_to_grank[j];
        }
        else {
	    /* Negative rank */
	    MPIR_ERROR_PUSH_ARG( &j );
	    return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );

	}
      }
  }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group_ptr->np, &(new_group_ptr->N2_next), 
		     &(new_group_ptr->N2_prev) );
  TR_POP;
  return (mpi_errno);
}
