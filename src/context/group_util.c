/*
 *  $Id: group_util.c,v 1.18 1994/09/30 22:11:51 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"


MPI_Group MPIR_CreateGroup( np )
int np;
{
    MPI_Group  new;
    int        i;

    new = NEW(struct MPIR_GROUP);
    if (!new) return 0;
    MPIR_SET_COOKIE(new,MPIR_GROUP_COOKIE)
    new->np             = np;
    if (np > 0) {
	new->lrank_to_grank = (int *) MALLOC( np * sizeof(int) );
	if (!new->lrank_to_grank) return 0;
	}
    else 
	new->lrank_to_grank = 0;
    new->set_mark   = (int *)0;
    new->local_rank = MPI_UNDEFINED;
    new->ref_count  = 1;
    new->permanent  = 0;
    MPIR_Powers_of_2 ( np, &(new->N2_next), &(new->N2_prev) );

    for (i=0; i<np; i++) 
	new->lrank_to_grank[i] = -1;

    return new;
}

void MPIR_FreeGroup( group )
MPI_Group group;
{
  if (group->lrank_to_grank) {
      FREE( group->lrank_to_grank );
      }
  if ( group->set_mark ) {
      FREE( group->set_mark );
      }
  FREE( group );
}

void MPIR_SetToIdentity( g )
MPI_Group g;
{
  int np, i;

  np = g->np;
  for (i=0; i<np; i++) 
    g->lrank_to_grank[i] = i;

  MPID_Myrank(MPI_COMM_WORLD->ADIctx,&(g->local_rank));
  if (g->local_rank > np)
    g->local_rank = MPI_UNDEFINED;
}

/*+

MPIR_Group_dup -

+*/
int MPIR_Group_dup( group, new_group )
MPI_Group group, *new_group;
{
  (*new_group) = group;
  if ( group != MPI_GROUP_NULL )
    group->ref_count++;
  return(MPI_SUCCESS);
}


/*+

MPIR_Dump_group - dump group information

+*/
int MPIR_Dump_group ( group )
MPI_Group group;
{
  int i, rank;
  MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

  printf ( "\t[%d] group       = %d\n", rank, group );
  if (group != NULL) {
    printf ( "\t[%d] np          = %d\n", rank, group->np );
    printf ( "\t[%d] local rank  = %d\n", rank, group->local_rank );
    printf ( "\t[%d] local rank -> global rank mapping\n", rank );
    for ( i=0; i<group->np; i++ )
      printf ( "\t [%d]   %d             %d\n", rank, i, group->lrank_to_grank[i] );
  }
  return MPI_SUCCESS;
}

/*+

MPIR_Dump_ranks - dump an array of ranks

+*/
int MPIR_Dump_ranks ( n, ranks )
int n, *ranks;
{
  int i;

  printf ( "\tnumber of ranks = %d\n", n );
  printf ( "\t n     rank\n" );
  for ( i=0; i<n; i++ )
    printf ( "\t %d      %d\n", i, ranks[i] );
  return MPI_SUCCESS;
}

/*+

MPIR_Dump_ranges - dump an array of ranges

+*/
int MPIR_Dump_ranges ( n, ranges )
int n, *ranges;
{
  int i;

  printf ( "\tnumber of ranges = %d\n", n );
  printf ( "\t first    last    stride\n" );
  for ( i=0; i<n; i++ )
  printf ( "\t %d      %d        %d       %d\n", i, ranges[i*3],
          ranges[(i*3)+1], ranges[(i*3)+2] );
  return MPI_SUCCESS;
}


/*+

MPIR_Powers_of_2 - given a number N, determine the previous and next
                   powers of 2

+*/
int MPIR_Powers_of_2 ( N, N2_next, N2_prev )
int  N;
int *N2_next, *N2_prev;
{
  int high     = 131072;
  int low      = 1;

  while( (high > N) && (low < N) ) {
    high >>= 1; low  <<= 1;
  }

  if(high <= N) {
    if(high == N)   /* no defect, power of 2! */
      (*N2_next) = N;
	else
      (*N2_next) = high << 1;
  }
  else {/* condition low >= N satisfied */
    if(low == N)	/* no defect, power of 2! */
      (*N2_next) = N;
	else
      (*N2_next) = low;
  }

  if( N == (*N2_next) ) /* power of 2 */
	(*N2_prev) = N;
  else
	(*N2_prev) = (*N2_next) >> 1;

  return (MPI_SUCCESS);
}

/*+

MPIR_Group_N2_prev - retrieve greatest power of two < size of group.

+*/
int MPIR_Group_N2_prev ( group, N2_prev )
MPI_Group  group;
int       *N2_prev;
{
  (*N2_prev) = group->N2_prev;
  return (MPI_SUCCESS);
}






