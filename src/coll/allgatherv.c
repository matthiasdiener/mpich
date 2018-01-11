/*
 *  $Id: allgatherv.c,v 1.19 1996/06/07 15:08:09 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

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
 'i'  specifies the displacement (relative to recvbuf ) at
which to place the incoming data from process  'i'  
. recvtype - data type of receive buffer elements (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

Notes:
 The MPI standard (1.0 and 1.1) says that 

 The jth block of data sent from 
 each proess is received by every process and placed in the jth block of the 
 buffer 'recvbuf'.  

 This is misleading; a better description is

 The block of data sent from the jth process is received by every
 process and placed in the jth block of the buffer 'recvbuf'.

 This text was suggested by Rajeev Thakur.

.N fortran

.N Errors
.N MPI_ERR_BUFFER
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
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
  MPIR_ERROR_DECL;

  /* Check for invalid arguments */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_COUNT(comm,sendcount) ||
      MPIR_TEST_DATATYPE(comm,sendtype) || MPIR_TEST_DATATYPE(comm,recvtype))
      return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLGATHERV" ); 

  MPIR_ERROR_PUSH(comm)
  mpi_errno = comm->collops->Allgatherv( sendbuf, sendcount,  sendtype, 
					 recvbuf,  recvcounts, displs,   
					 recvtype, comm );
  MPIR_ERROR_POP(comm);
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_ALLGATHERV");
}
