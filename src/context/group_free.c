/*
 *  $Id: group_free.c,v 1.12 1994/09/13 21:48:24 gropp Exp $
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
On output, group is set to MPI_GROUP_NULL.

@*/
int MPI_Group_free ( group )
MPI_Group *group;
{
  int errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_ARG(group) )
	return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_GROUP_FREE" );

  /* Free null groups succeeds silently */
  if ( (*group) == MPI_GROUP_NULL )
	return (MPI_SUCCESS);
	 
  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*group)->permanent == 1 ) && (*group)->ref_count <= 1 && 
          (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_PERM_KEY,
					  "Error in MPI_GROUP_FREE" );

  /* Free group */
  if ( (*group)->ref_count <= 1 ) 
	MPIR_FreeGroup(*group);
  else 
	(*group)->ref_count--;
  /* This could be dangerous if the object is MPI_GROUP_EMPTY and not just
     a copy of it.... */
  (*group) = MPI_GROUP_NULL;

  return (errno);
}
