/*
 *  $Id: group_free.c,v 1.15 1996/04/12 14:09:43 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@

MPI_Group_free - Frees a group

Input Parameter
. group - group (handle) 

Notes:
On output, group is set to 'MPI_GROUP_NULL'.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
.N MPI_ERR_PERM_GROUP
@*/
int MPI_Group_free ( group )
MPI_Group *group;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_ARG(group) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_GROUP_FREE" );

  /* Free null groups succeeds silently */
  if ( (*group) == MPI_GROUP_NULL )
	return (MPI_SUCCESS);
	 
  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*group)->permanent == 1 ) && (*group)->ref_count <= 1 && 
          (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_PERM_GROUP,
					  "Error in MPI_GROUP_FREE" );

  /* Free group */
  if ( (*group)->ref_count <= 1 ) 
	MPIR_FreeGroup(*group);
  else 
	(*group)->ref_count--;
  /* This could be dangerous if the object is MPI_GROUP_EMPTY and not just
     a copy of it.... */
  (*group) = MPI_GROUP_NULL;

  return (mpi_errno);
}
