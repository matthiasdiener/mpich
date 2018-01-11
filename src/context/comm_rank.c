/*
 *  $Id: comm_rank.c,v 1.13 1994/12/15 16:26:19 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: comm_rank.c,v 1.13 1994/12/15 16:26:19 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@

MPI_Comm_rank - Determines the rank of the calling process in the communicator

Input Parameters:
. comm - communicator (handle) 
Output Parameter:
. rank - rank of the calling process in group of  comm  (integer) 

@*/
int MPI_Comm_rank ( comm, rank )
MPI_Comm  comm;
int      *rank;
{
  int mpi_errno;
  if ( MPIR_TEST_COMM(comm,comm) ) {
    (*rank) = MPI_UNDEFINED;
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_COMM_RANK" );
  }
  else 
    (*rank) = comm->local_group->local_rank;

  return (MPI_SUCCESS);
}
