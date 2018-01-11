/*
 *  $Id: comm_test_ic.c,v 1.7 1994/07/13 15:46:17 lusk Exp $
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
  int errno = MPI_SUCCESS;

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(flag) )
	return MPIR_ERROR( comm, errno, "Error in MPI_COMM_TEST_INTER" );
  
  /* Set the flag */
  if (comm->comm_type == MPIR_INTER)
	(*flag) = 1; 
  else
	(*flag) = 0; 

  return (errno);
}
