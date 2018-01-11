/*
 *  $Id: group_union.c,v 1.16 1994/12/15 16:35:50 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Group_union - Produces a group by combining two groups

Input Parameters:
. group1 - first group (handle) 
. group2 - second group (handle) 

Output Parameter:
. newgroup - union group (handle) 

@*/
int MPI_Group_union ( group1, group2, group_out )
MPI_Group group1, group2, *group_out;
{
  int        i, j, global_rank;
  MPI_Group  new_group;
  int        n;
  int        mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group1) ||
       MPIR_TEST_GROUP(MPI_COMM_WORLD,group2))
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_GROUP_UNION" );
  
  /* Check for EMPTY groups */
  if ( (group1 == MPI_GROUP_EMPTY) && (group2 == MPI_GROUP_EMPTY) ) {
	(void) MPIR_Group_dup ( MPI_GROUP_EMPTY, group_out );
    return (mpi_errno);
  }
  if ( group1 == MPI_GROUP_EMPTY ) {
    (void) MPIR_Group_dup ( group2, group_out );
    return (mpi_errno);
  }
  if ( group2 == MPI_GROUP_EMPTY ) {
    (void) MPIR_Group_dup ( group1, group_out );
    return (mpi_errno);
  }
  
  /* Create the new group */
  new_group = (*group_out) = NEW(struct MPIR_GROUP);
  if(!new_group)
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_UNION" );
  MPIR_SET_COOKIE(new_group,MPIR_GROUP_COOKIE)
  new_group->ref_count     = 1;
  new_group->permanent     = 0;
  new_group->local_rank    = group1->local_rank;
  new_group->set_mark      = (int *)0;
  
  /* Set the number in the union */
  n = group1->np + group2->np;

  /* Allocate set marking space for group2 if necessary */
  if (group2->set_mark == NULL)
    group2->set_mark = (int *) MALLOC( group2->np * sizeof(int) );

  /* Mark the union */
  for ( j=0; j<group2->np; j++ ) {
    group2->set_mark[j] = MPIR_MARKED;
    for ( i=0; i<group1->np; i++ ) 
      if ( group1->lrank_to_grank[i] == group2->lrank_to_grank[j] ) {
        group2->set_mark[j] = MPIR_UNMARKED;
        n--;
        break;
      }
  }
  
  /* Alloc the memory */
  new_group->np             = n;
  new_group->lrank_to_grank = (int *) MALLOC( n * sizeof(int) );
  if (!new_group->lrank_to_grank) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_GROUP_UNION" );
  }
  
  /* Fill in the space */
  n = group1->np;
  memcpy(new_group->lrank_to_grank,group1->lrank_to_grank,n*sizeof(int));
  for ( j=0; j<group2->np; j++ ) 
    if ( (group2->set_mark[j]==MPIR_MARKED) && (n < new_group->np) ) 
      new_group->lrank_to_grank[n++] = group2->lrank_to_grank[j];
  
  /* Find the local rank only if local rank not defined in group 1 */
  if ( new_group->local_rank == MPI_UNDEFINED ) {
    MPID_Myrank(MPI_COMM_WORLD->ADIctx,&global_rank);
    for( i=group1->np; i<new_group->np; i++ )
      if ( global_rank == new_group->lrank_to_grank[i] ) {
        new_group->local_rank = i;
        break;
      }
  }

  /* Determine the previous and next powers of 2 */
  MPIR_Powers_of_2 ( new_group->np, &(new_group->N2_next), &(new_group->N2_prev) );

  return (mpi_errno);
}



