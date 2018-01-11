/*
 *  $Id: gatherv.c,v 1.26 1997/01/07 01:47:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Gatherv - Gathers into specified locations from all processes in a group

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcounts - integer array (of length group size) 
containing the number of elements that are received from each process
(significant only at 'root') 
. displs - integer array (of length group size). Entry 
 'i'  specifies the displacement relative to recvbuf  at
which to place the incoming data from process  'i'  (significant only
at root) 
. recvtype - data type of recv buffer elements 
(significant only at 'root') (handle) 
. root - rank of receiving process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, significant only at 'root') 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Gatherv ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm )
void             *sendbuf;
int               sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *stype_ptr, *rtype_ptr;
  MPIR_ERROR_DECL;
  static char myname[] = "MPI_GATHERV";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr, myname);

  stype_ptr = MPIR_GET_DTYPE_PTR(sendtype);
  MPIR_TEST_DTYPE(sendtype,stype_ptr,comm_ptr, myname );

  /* rtype is significant only at root */
  rtype_ptr = MPIR_GET_DTYPE_PTR(recvtype);
  MPIR_TEST_DTYPE(recvtype,rtype_ptr,comm_ptr, myname );

  if ( MPIR_TEST_COUNT(comm,sendcnt) ) 
    return MPIR_ERROR(comm_ptr, mpi_errno, myname );

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Gatherv( sendbuf, sendcnt,  stype_ptr, 
                  recvbuf, recvcnts, displs, rtype_ptr, 
                  root, comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
