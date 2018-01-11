/*
 *  $Id: comm_rgroup.c,v 1.4 1994/07/13 15:45:51 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_remote_group - Accesses the remote group associated with 
                        the given inter-communicator

Input Parameter:
. comm - Communicator

Output Parameter:
. group - remote group of communicator
@*/
int MPI_Comm_remote_group ( comm, group )
MPI_Comm comm;
MPI_Group *group;
{
  int errno;
  if (MPIR_TEST_COMM(comm,comm)) {
    (*group) = MPI_GROUP_NULL;
    return MPIR_ERROR( MPI_COMM_WORLD, errno, 
		       "Error in MPI_COMM_REMOTE_GROUP" );
  }
  else {
    MPIR_Group_dup( comm->group, group );
	return (MPI_SUCCESS);
  }
}
