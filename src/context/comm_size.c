/*
 *  $Id: comm_size.c,v 1.13 1995/04/23 18:40:40 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*@

MPI_Comm_size - Determines the size of the group associated with a communictor

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. size - number of processes in the group of comm  (integer) 

Notes:
   MPI_COMM_NULL is `not` considered a valid argument to this function.
@*/
int MPI_Comm_size ( comm, size )
MPI_Comm comm;
int *size;
{
  int mpi_errno;
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(size) ) {
    if (size) (*size) = MPI_UNDEFINED;
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_COMM_SIZE" );
  }
  else 
    (*size) = comm->local_group->np;

  return (MPI_SUCCESS);
}
