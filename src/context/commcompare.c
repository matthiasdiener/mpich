/*
 *  $Id: commcompare.c,v 1.5 1997/01/07 01:47:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_compare - Compares two communicators

Input Parameters:
. comm1 - comm1 (handle) 
. comm2 - comm2 (handle) 

Output Parameter:
. result - integer which is 'MPI_IDENT' if the contexts and groups are the
same, 'MPI_CONGRUENT' if different contexts but identical groups, 'MPI_SIMILAR'
if different contexts but similar groups, and 'MPI_UNEQUAL' otherwise

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
@*/
int MPI_Comm_compare ( comm1, comm2, result )
MPI_Comm  comm1;
MPI_Comm  comm2;
int       *result;
{
  int       mpi_errno = MPI_SUCCESS;
  MPI_Group group1, group2;
  struct MPIR_COMMUNICATOR *comm1_ptr, *comm2_ptr;
  static char myname[] = "MPI_COMM_COMPARE";

  comm1_ptr = MPIR_GET_COMM_PTR(comm1);
  MPIR_TEST_MPI_COMM(comm1,comm1_ptr,comm1_ptr,myname);
  comm2_ptr = MPIR_GET_COMM_PTR(comm2);
  MPIR_TEST_MPI_COMM(comm2,comm2_ptr,comm2_ptr,myname);

  /* Check for bad arguments */
  if ( ( (result == (int *)0)     && (mpi_errno = MPI_ERR_ARG) ) )
    return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

  if (!comm1 && !comm2) {
      *result = MPI_IDENT;
      return mpi_errno;
      }
  if ((!comm1 && comm2) || (!comm2 && comm1)) {
      *result = MPI_UNEQUAL;
      return mpi_errno;
      }
      
  /* Are they the same kind of communicator */
  if (comm1_ptr->comm_type != comm2_ptr->comm_type) {
	(*result) = MPI_UNEQUAL;
	return (mpi_errno);
  }

  /* See if they are identical */
  if (comm1 == comm2) {
	(*result) = MPI_IDENT;
	return (mpi_errno);
  }
	
  /* Comparison for intra-communicators */
  if (comm1_ptr->comm_type == MPIR_INTRA) {

	/* Get the groups and see what their relationship is */
	MPI_Comm_group (comm1, &group1);
	MPI_Comm_group (comm2, &group2);
	MPI_Group_compare ( group1, group2, result );

	/* They can't be identical since they're not the same
	   handle, they are congruent instead */
	if ((*result) == MPI_IDENT)
	  (*result) = MPI_CONGRUENT;

	/* Free the groups */
	MPI_Group_free (&group1);
	MPI_Group_free (&group2);
  }
  
  /* Comparison for inter-communicators */
  else {
	int       lresult, rresult;
	MPI_Group rgroup1, rgroup2;
	
	/* Get the groups and see what their relationship is */
	MPI_Comm_group (comm1, &group1);
	MPI_Comm_group (comm2, &group2);
	MPI_Group_compare ( group1, group2, &lresult );

	MPI_Comm_remote_group (comm1, &rgroup1);
	MPI_Comm_remote_group (comm2, &rgroup2);
	MPI_Group_compare ( rgroup1, rgroup2, &rresult );

	/* Choose the result that is "least" strong. This works 
	   due to the ordering of result types in mpi.h */
	(*result) = (rresult > lresult) ? rresult : lresult;

	/* They can't be identical since they're not the same
	   handle, they are congruent instead */
	if ((*result) == MPI_IDENT)
	  (*result) = MPI_CONGRUENT;

	/* Free the groups */
	MPI_Group_free (&group1);
	MPI_Group_free (&group2);
	MPI_Group_free (&rgroup1);
	MPI_Group_free (&rgroup2);
  }

  return (mpi_errno);
}
