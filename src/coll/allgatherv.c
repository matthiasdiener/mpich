/*
 *  $Id: allgatherv.c,v 1.16 1995/06/21 03:08:59 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: allgatherv.c,v 1.16 1995/06/21 03:08:59 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Allgatherv - Gathers data from all tasks and deliver it to all

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcounts - integer array (of length group size) 
containing the number of elements that are received from each process 
. displs - integer array (of length group size). Entry 
 i  specifies the displacement (relative to recvbuf ) at
which to place the incoming data from process  i  
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 
@*/
int MPI_Allgatherv ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcounts, displs,   recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcounts;
int              *displs;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcount) ||
      MPIR_TEST_DATATYPE(comm,sendtype) || MPIR_TEST_DATATYPE(comm,recvtype))
      return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLGATHERV" ); 

  return comm->collops->Allgatherv( sendbuf, sendcount,  sendtype, 
				   recvbuf,  recvcounts, displs,   
				   recvtype, comm );
}
