/*
 *  $Id: groupcompare.c,v 1.2 1998/04/28 20:58:21 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_compare - Compares two groups

Input Parameters:
+ group1 - group1 (handle) 
- group2 - group2 (handle) 

Output Parameter:
. result - integer which is 'MPI_IDENT' if the order and members of
the two groups are the same, 'MPI_SIMILAR' if only the members are the same,
and 'MPI_UNEQUAL' otherwise

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_ARG
@*/
int MPI_Group_compare ( group1, group2, result )
MPI_Group  group1;
MPI_Group  group2;
int       *result;
{
  int       mpi_errno = MPI_SUCCESS;
  int       size1, size2;
  struct MPIR_GROUP *group1_ptr, *group2_ptr;
  MPI_Group group_int;
  int       size_int, i;
  static char myname[] = "MPI_GROUP_COMPARE";

  TR_PUSH(myname);

  group1_ptr = MPIR_GET_GROUP_PTR(group1);
  MPIR_TEST_MPI_GROUP(group1,group1_ptr,MPIR_COMM_WORLD,myname);

  group2_ptr = MPIR_GET_GROUP_PTR(group2);
  MPIR_TEST_MPI_GROUP(grou2p,group2_ptr,MPIR_COMM_WORLD,myname);

  if ( MPIR_TEST_ARG(result))
    return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  /* See if their sizes are equal */
  MPI_Group_size ( group1, &size1 );
  MPI_Group_size ( group2, &size2 );
  if ( size1 != size2 ) {
	(*result) = MPI_UNEQUAL;
	TR_POP;
	return MPI_SUCCESS;
  }

  /* Is their intersection the same size as the original group */
  MPI_Group_intersection ( group1, group2, &group_int );
  MPI_Group_size ( group_int, &size_int );
  MPI_Group_free ( &group_int );
  if ( size_int != size1 ) {
	(*result) = MPI_UNEQUAL;
	TR_POP;
	return MPI_SUCCESS;
  }

  /* Do a 1-1 comparison */
  (*result) = MPI_SIMILAR;
  for ( i=0; i<size1; i++ )
      if ( group1_ptr->lrank_to_grank[i] != group2_ptr->lrank_to_grank[i] ) {
	  TR_POP;
	  return MPI_SUCCESS;
      }
  (*result) = MPI_IDENT;

  TR_POP;
  return MPI_SUCCESS;
}
