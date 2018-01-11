/*
 *  $Id: scatter.c,v 1.4 1998/04/28 18:51:06 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Scatter - Sends data from one task to all other tasks in a group

Input Parameters:
+ sendbuf - address of send buffer (choice, significant 
only at 'root') 
. sendcount - number of elements sent to each process 
(integer, significant only at 'root') 
. sendtype - data type of send buffer elements (significant only at 'root') 
(handle) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - data type of receive buffer elements (handle) 
. root - rank of sending process (integer) 
- comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
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
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *stype_ptr;
  struct MPIR_DATATYPE     *rtype_ptr;
  static char myname[] = "MPI_SCATTER";
  MPIR_ERROR_DECL;  

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  /* Significant only at root */
  stype_ptr = MPIR_GET_DTYPE_PTR(sendtype);
  MPIR_TEST_DTYPE(sendtype,stype_ptr,comm_ptr,myname);

  rtype_ptr = MPIR_GET_DTYPE_PTR(recvtype);
  MPIR_TEST_DTYPE(recvtype,rtype_ptr,comm_ptr,myname);

#ifdef FOO
/* Only check for matching signature */
  /* Check for mismatched receive/send types - Debbie Swider 11/20/97 */
  if (recvtype != sendtype)
    return MPIR_ERROR(comm_ptr, MPI_ERR_TYPE, myname);
#endif

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Scatter(sendbuf, sendcnt, stype_ptr, 
				recvbuf, recvcnt, rtype_ptr, 
				root, comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
