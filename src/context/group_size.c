/*
 *  $Id: group_size.c,v 1.15 1997/01/07 01:47:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Group_size - Returns the size of a group

Input Parameters:
. group - group (handle) 
Output Parameter:
. size - number of processes in the group (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_GROUP
.N MPI_ERR_ARG

@*/
int MPI_Group_size ( group, size )
MPI_Group group;
int *size;
{
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_GROUP *group_ptr;
  static char myname[] = "MPI_GROUP_SIZE";

  TR_PUSH(myname);

  group_ptr = MPIR_GET_GROUP_PTR(group);
  MPIR_TEST_MPI_GROUP(group,group_ptr,MPIR_COMM_WORLD,myname);

  /* Check for invalid arguments */
  if ( MPIR_TEST_ARG(size))
      return MPIR_ERROR(MPIR_COMM_WORLD,mpi_errno,myname);

  /* Get the size of the group */
  (*size) = group_ptr->np;

  TR_POP;
  return (mpi_errno);
}

