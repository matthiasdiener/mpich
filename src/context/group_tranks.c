/*
 *  $Id: group_tranks.c,v 1.3 1998/11/28 22:08:56 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_translate_ranks - Translates the ranks of processes in one group to 
                            those in another group

Input Parameters:
+ group1 - group1 (handle) 
. n - number of ranks in  'ranks1' and 'ranks2'  arrays (integer) 
. ranks1 - array of zero or more valid ranks in 'group1' 
- group2 - group2 (handle) 

Output Parameter:
. ranks2 - array of corresponding ranks in group2,  'MPI_UNDEFINED'  when no 
correspondence exists. 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_ARG
.N MPI_ERR_RANK

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
  struct MPIR_GROUP *group_a_ptr, *group_b_ptr;
  int mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_GROUP_TRANSLATE_RANKS";

  TR_PUSH(myname);

  group_a_ptr = MPIR_GET_GROUP_PTR(group_a);
  MPIR_TEST_MPI_GROUP(group_a,group_a_ptr,MPIR_COMM_WORLD,myname);

  group_b_ptr = MPIR_GET_GROUP_PTR(group_b);
  MPIR_TEST_MPI_GROUP(group_b,group_b_ptr,MPIR_COMM_WORLD,myname);

  /* Check for bad arguments */
  if ( ( (n       <= 0)              && (mpi_errno = MPI_ERR_ARG) )   ||
       MPIR_TEST_ARG(ranks_a) || MPIR_TEST_ARG(ranks_b))
    return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  /* Set ranks_b array to MPI_UNDEFINED */
  for ( i=0; i<n; i++ )
    ranks_b[i] = MPI_UNDEFINED;
 
  /* Translate the ranks in ranks_a to ranks_b */
  for ( i=0 ; i<n; i++ ) {
      if ( ((rank_a = ranks_a[i]) >= group_a_ptr->np) || (rank_a < 0) ) {
	  MPIR_ERROR_PUSH_ARG(&rank_a);
	  return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_RANK, myname );
      }
    pid_a = group_a_ptr->lrank_to_grank[rank_a];
    for ( j=0; j<group_b_ptr->np; j++ ) 
      if ( pid_a == group_b_ptr->lrank_to_grank[j] ) {
        ranks_b[i] = j;
        break;
      }
  }
  TR_POP;
  return MPI_SUCCESS;
}
