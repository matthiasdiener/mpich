/*
 *  $Id: group_tranks.c,v 1.10 1994/07/13 15:47:58 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_translate_ranks - Translates the ranks of processes in one group to 
                            those in another group

Input Parameters:
. group1 - group1 (handle) 
. n - number of ranks in  ranks1 and ranks2  arrays (integer) 
. ranks1 - array of zero or more valid ranks in group1 
. group2 - group2 (handle) 

Output Parameter:
. ranks2 - array of corresponding ranks in group2,  MPI_UNDEFINED  when no 
correspondence exists. 

@*/
int MPI_Group_translate_ranks ( group_a, n, ranks_a, group_b, ranks_b )
MPI_Group group_a;
int n;
int       *ranks_a;
MPI_Group  group_b;
int       *ranks_b;
{
  int i, j;
  int pid_a, rank_a;
  int errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group_a) ||
       MPIR_TEST_GROUP(MPI_COMM_WORLD,group_b) || 
	   ( (n       <= 0)              && (errno = MPI_ERR_ARG) )   ||
       MPIR_TEST_ARG(ranks_a) || MPIR_TEST_ARG(ranks_b))
    return MPIR_ERROR( MPI_COMM_WORLD, errno, 
				  "Error in MPI_GROUP_TRANSLATE_RANKS" );

  /* Set ranks_b array to MPI_UNDEFINED */
  for ( i=0; i<n; i++ )
    ranks_b[i] = MPI_UNDEFINED;
 
  /* Translate the ranks in ranks_a to ranks_b */
  for ( i=0 ; i<n; i++ ) {
    if ( ((rank_a = ranks_a[i]) >= group_a->np) || (rank_a < 0) ) 
      break;
    pid_a = group_a->lrank_to_grank[rank_a];
    for ( j=0; j<group_b->np; j++ ) 
      if ( pid_a == group_b->lrank_to_grank[j] ) {
        ranks_b[i] = j;
        break;
      }
  }
  return (errno);
}
