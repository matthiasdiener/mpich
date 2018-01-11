/*
 *  $Id: alltoallv.c,v 1.22 1995/06/21 03:09:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: alltoallv.c,v 1.22 1995/06/21 03:09:32 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@

MPI_Alltoallv - Sends data from all to all processes, with a displacement

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcounts - integer array equal to the group size 
specifying the number of elements to send to each processor 
. sdispls - integer array (of length group size). Entry 
 j  specifies the displacement (relative to sendbuf  from
which to take the outgoing data destined for process  j  
. sendtype - data type of send buffer elements (handle) 
. recvcounts - integer array equal to the group size 
specifying the maximum number of elements that can be received from
each processor 
. rdispls - integer array (of length group size). Entry 
 i  specifies the displacement (relative to recvbuf  at
which to place the incoming data from process  i  
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 
@*/
int MPI_Alltoallv ( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm )
void             *sendbuf;
int              *sendcnts;
int              *sdispls;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *rdispls; 
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  
  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,sendtype) ||
       MPIR_TEST_DATATYPE(comm,recvtype))
	return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLTOALLV" ); 

  return comm->collops->Alltoallv( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm );
}
