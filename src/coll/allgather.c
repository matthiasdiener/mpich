/*
 *  $Id: allgather.c,v 1.18 1995/05/16 18:10:23 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: allgather.c,v 1.18 1995/05/16 18:10:23 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Allgather - Gathers data from all tasks and distribute it to all 

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements received from any process (integer) 
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 
@*/
int MPI_Allgather ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcount;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int size, rank, root;
  int mpi_errno = MPI_SUCCESS;
  int flag;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcount) ||
       MPIR_TEST_COUNT(comm,recvcount) || 
       MPIR_TEST_DATATYPE(comm,sendtype) || 
       MPIR_TEST_DATATYPE(comm,recvtype)) 
      return MPIR_ERROR( comm, mpi_errno, "Error in MPI_ALLGATHER" ); 
  
  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
			  "Inter-communicator invalid in MPI_ALLGATHER");

  /* Get the size of the communicator */
  MPI_Comm_size ( comm, &size );

  /* Do a gather for each process in the communicator */
  /* This is a sorry way to do this, but for now ... */
  for (root=0; root<size; root++) {
    mpi_errno = MPI_Gather(sendbuf,sendcount,sendtype,
			   recvbuf,recvcount,recvtype,root,comm);
    if (mpi_errno) break;
    }

  return (mpi_errno);
}


