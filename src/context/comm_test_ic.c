/*
 *  $Id: comm_test_ic.c,v 1.8 1994/12/15 16:40:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_test_inter - Tests to see if a comm is an inter-communicator

Input Parameter:
. comm - communicator (handle) 
Output Parameter:
. flag - (logical) 

@*/
int MPI_Comm_test_inter ( comm, flag )
MPI_Comm  comm;
int      *flag;
{
  int mpi_errno = MPI_SUCCESS;

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(flag) )
	return MPIR_ERROR( comm, mpi_errno, "Error in MPI_COMM_TEST_INTER" );
  
  /* Set the flag */
  if (comm->comm_type == MPIR_INTER)
	(*flag) = 1; 
  else
	(*flag) = 0; 

  return (mpi_errno);
}
