/*
 *  $Id: scatter.c,v 1.23 1995/12/21 22:17:36 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: scatter.c,v 1.23 1995/12/21 22:17:36 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Scatter - Sends data from one task to all other tasks in a group

Input Parameters:
. sendbuf - address of send buffer (choice, significant 
only at 'root') 
. sendcount - number of elements sent to each process 
(integer, significant only at 'root') 
. sendtype - data type of send buffer elements (significant only at 'root') 
(handle) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - data type of receive buffer elements (handle) 
. root - rank of sending process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

.N fortran
@*/
int MPI_Scatter ( sendbuf, sendcnt, sendtype, 
		  recvbuf, recvcnt, recvtype, 
		  root, comm )
void             *sendbuf;
int               sendcnt;
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

  return comm->collops->Scatter(sendbuf, sendcnt, sendtype, 
				recvbuf, recvcnt, recvtype, 
				root, comm );
}
