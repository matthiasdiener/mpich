/*
 *  $Id: group_inter.c,v 1.18 1996/04/12 14:11:11 gropp Exp $
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

MPI_Group_intersection - Produces a group as the intersection of two existing
                         groups

Input Parameters:
. group1 - first group (handle) 
. group2 - second group (handle) 

Output Parameter:
. newgroup - intersection group (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Group_free
@*/
int MPI_Group_intersection ( group1, group2, group_out )
MPI_Group group1, group2, *group_out;
{
  int        i, j, global_rank;
  MPI_Group  new_group;
  int        n;
  int        mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group1) ||
       MPIR_TEST_GROUP(MPI_COMM_WORLD,group2) )
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
					  "Error in MPI_GROUP_INTERSECTION" );

  /* Check for EMPTY groups */
  if ( (group1 == MPI_GROUP_EMPTY) || (group2 == MPI_GROUP_EMPTY) ) {
      (void) MPIR_Group_dup ( MPI_GROUP_EMPTY, group_out );
      return (mpi_errno);
  }
  
  /* Create the new group */
  MPIR_ALLOC(new_group,NEW(struct MPIR_GROUP),MPI_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "Out of space in MPI_GROUP_INTERSECTION" );
  *group_out = new_group;
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count     = 1;
  new_group->permanent     = 0;
  new_group->local_rank    = MPI_UNDEFINED;
  new_group->set_mark      = (int *)0;

  /* Set the number in the intersection */
  n = 0;

  /* Allocate set marking space for group1 if necessary */
  if (group1->set_mark == NULL) {
      MPIR_ALLOC(group1->set_mark,(int *) MALLOC( group1->np * sizeof(int) ),
		 MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		 "Out of space in MPI_GROUP_INTERSECTION" );
  }

  /* Mark the intersection */
  for ( i=0; i<group1->np; i++ ) {
    group1->set_mark[i] = MPIR_UNMARKED;
    for ( j=0; j<group2->np; j++ ) 
      if ( group1->lrank_to_grank[i] == group2->lrank_to_grank[j] ) {
        group1->set_mark[i] = MPIR_MARKED;
        n++;
        break;
      }
  }

  /* If there is a null intersection */
  if ( n <= 0 ) {
	FREE( new_group );
	(void) MPIR_Group_dup ( MPI_GROUP_EMPTY, group_out );
	return (mpi_errno);
  }

  /* Alloc memory for lrank_to_grank array */
  new_group->np             = n;
  new_group->lrank_to_grank = (int *) MALLOC( n * sizeof(int) );
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_INTERSECTION" );
  }
    
  /* Fill in the space */
  for ( n=0, i=0; i<group1->np; i++ ) 
    if ( (group1->set_mark[i]==MPIR_MARKED) && (n < new_group->np) ) 
      new_group->lrank_to_grank[n++] = group1->lrank_to_grank[i];

  /* Find the local rank */
#ifdef MPI_ADI2
  global_rank = MPID_MyWorldRank;
#else
  MPID_Myrank(MPI_COMM_WORLD->ADIctx, &global_rank);
#endif
  for( i=0; i<new_group->np; i++ )
    if ( global_rank == new_group->lrank_to_grank[i] ) {
      new_group->local_rank = i;
      break;
    }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next),
		     &(new_group->N2_prev) );

  return (mpi_errno);
}





