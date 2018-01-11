/*
 *  $Id: scatterv.c,v 1.24 1995/06/30 13:49:36 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: scatterv.c,v 1.24 1995/06/30 13:49:36 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Scatterv - Scatters a buffer in parts to all tasks in a group

Input Parameters:
. sendbuf - address of send buffer (choice, significant only at root) 
. sendcounts - integer array (of length group size) 
specifying the number of elements to send to each processor  
. displs - integer array (of length group size). Entry 
 i  specifies the displacement (relative to sendbuf  from
which to take the outgoing data to process  i  
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - data type of receive buffer elements (handle) 
. root - rank of sending process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

@*/
int MPI_Scatterv ( sendbuf, sendcnts, displs, sendtype, 
                   recvbuf, recvcnt,  recvtype, 
                   root, comm )
void             *sendbuf;
int              *sendcnts;
int              *displs;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcnt;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,sendtype)) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_SCATTER" );

  return comm->collops->Scatterv( sendbuf, sendcnts, displs, sendtype, 
				 recvbuf, recvcnt,  recvtype, 
				 root, comm );
}
