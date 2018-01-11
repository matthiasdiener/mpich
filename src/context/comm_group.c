/*
 *  $Id: comm_group.c,v 1.9 1994/07/13 15:45:38 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_group - Accesses the group associated with given communicator

Input Parameter:
. comm - Communicator

Output Parameter:
. group - Group in communicator

@*/
int MPI_Comm_group ( comm, group )
MPI_Comm comm;
MPI_Group *group;
{
  int errno;
  if ( MPIR_TEST_COMM(comm,comm) ) {
    (*group) = MPI_GROUP_NULL;
    return MPIR_ERROR( MPI_COMM_WORLD, errno, "Error in MPI_COMM_GROUP" );
  }
  else {
    MPIR_Group_dup( comm->local_group, group );
	return (MPI_SUCCESS);
  }
}
