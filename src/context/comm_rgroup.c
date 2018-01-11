/*
 *  $Id: comm_rgroup.c,v 1.5 1994/12/15 16:37:56 gropp Exp $
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
  int mpi_errno;
  if (MPIR_TEST_COMM(comm,comm)) {
    (*group) = MPI_GROUP_NULL;
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
		       "Error in MPI_COMM_REMOTE_GROUP" );
  }
  else {
    MPIR_Group_dup( comm->group, group );
	return (MPI_SUCCESS);
  }
}
