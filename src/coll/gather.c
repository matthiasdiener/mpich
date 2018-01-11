/*
 *  $Id: gather.c,v 1.4 1998/09/22 15:49:42 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Gather - Gathers together values from a group of processes
 
Input Parameters:
+ sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements for any single receive (integer, 
significant only at root) 
. recvtype - data type of recv buffer elements 
(significant only at root) (handle) 
. root - rank of receiving process (integer) 
- comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, significant only at 'root') 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Gather ( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, 
		 root, comm )
void             *sendbuf;
int               sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcount;
MPI_Datatype      recvtype;
int               root;
MPI_Comm          comm;
{
  int        mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *stype_ptr, *rtype_ptr;
  MPIR_ERROR_DECL;
  static char myname[] = "MPI_GATHER";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr, myname );

  stype_ptr = MPIR_GET_DTYPE_PTR(sendtype);
  MPIR_TEST_DTYPE(sendtype,stype_ptr,comm_ptr, myname );

  rtype_ptr = MPIR_GET_DTYPE_PTR(recvtype);
  MPIR_TEST_DTYPE(recvtype,rtype_ptr,comm_ptr, myname );

  if ( MPIR_TEST_COUNT(comm,sendcnt) ) 
    return MPIR_ERROR(comm_ptr, mpi_errno, myname );

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Gather(sendbuf, sendcnt, stype_ptr, 
			       recvbuf, recvcount, rtype_ptr, 
			       root, comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno, myname );
}
