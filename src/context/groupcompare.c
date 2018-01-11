/*
 *  $Id: groupcompare.c,v 1.2 1995/12/21 22:11:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_compare - Compares two groups

Input Parameters:
. group1 - group1 (handle) 
. group2 - group2 (handle) 

Output Parameter:
. result - integer which is 'MPI_IDENT' if the order and members of
the two groups are the same, 'MPI_SIMILAR' if only the members are the same,
and 'MPI_UNEQUAL' otherwise

.N fortran
@*/
int MPI_Group_compare ( group1, group2, result )
MPI_Group  group1;
MPI_Group  group2;
int       *result;
{
  int       mpi_errno = MPI_SUCCESS;
  int       size1, size2;
  MPI_Group group_int;
  int       size_int, i;

  if ( MPIR_TEST_GROUP(MPI_COMM_WORLD,group1) ||
       MPIR_TEST_GROUP(MPI_COMM_WORLD,group2) ||
       MPIR_TEST_ARG(result))
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
		       "Error in MPI_GROUP_COMPARE" );

  /* See if their sizes are equal */
  MPI_Group_size ( group1, &size1 );
  MPI_Group_size ( group2, &size2 );
  if ( size1 != size2 ) {
	(*result) = MPI_UNEQUAL;
	return (mpi_errno);
  }

  /* Is their intersection the same size as the original group */
  MPI_Group_intersection ( group1, group2, &group_int );
  MPI_Group_size ( group_int, &size_int );
  MPI_Group_free ( &group_int );
  if ( size_int != size1 ) {
	(*result) = MPI_UNEQUAL;
	return (mpi_errno);
  }

  /* Do a 1-1 comparison */
  (*result) = MPI_SIMILAR;
  for ( i=0; i<size1; i++ )
	if ( group1->lrank_to_grank[i] != group2->lrank_to_grank[i] ) 
	  return (mpi_errno);
  (*result) = MPI_IDENT;
  return (mpi_errno);
}
