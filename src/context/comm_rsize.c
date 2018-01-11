/*
 *  $Id: comm_rsize.c,v 1.6 1995/12/21 22:06:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*@

MPI_Comm_remote_size - Determines the size of the remote group 
                       associated with an inter-communictor

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. size - number of processes in the group of 'comm'  (integer) 

.N fortran
@*/
int MPI_Comm_remote_size ( comm, size )
MPI_Comm  comm;
int      *size;
{
  int mpi_errno;
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(size)) {
    if (size) (*size) = MPI_UNDEFINED;
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
		      "Error in MPI_COMM_REMOTE_SIZE" );
  }
  else 
    (*size) = comm->group->np;

  return (MPI_SUCCESS);
}
