/*
 *  $Id: alltoall.c,v 1.4 1998/04/28 18:50:44 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Alltoall - Sends data from all to all processes

Input Parameters:
+ sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements to send to each process (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements received from any process (integer) 
. recvtype - data type of receive buffer elements (handle) 
- comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice) 

.N fortran

.N Errors
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
@*/
int MPI_Alltoall( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int               recvcnt;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
  int          mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *stype_ptr, *rtype_ptr;
  MPIR_ERROR_DECL;
  static char myname[] = "MPI_ALLTOALL";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  stype_ptr = MPIR_GET_DTYPE_PTR(sendtype);
  MPIR_TEST_DTYPE(sendtype,stype_ptr,comm_ptr, myname );

  rtype_ptr = MPIR_GET_DTYPE_PTR(recvtype);
  MPIR_TEST_DTYPE(recvtype,rtype_ptr,comm_ptr, myname );
 
  /* Check for  mismatched receive/send types - Debbie Swider 11/20/97 */
  if (recvtype != sendtype)
      return MPIR_ERROR(comm_ptr, MPI_ERR_TYPE, myname);

  /* Check for invalid arguments */
  if ( MPIR_TEST_COUNT(comm,sendcount) ||
       MPIR_TEST_COUNT(comm,recvcnt) )
	return MPIR_ERROR(comm_ptr, mpi_errno, myname ); 
  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Alltoall(sendbuf, sendcount, stype_ptr, 
                  recvbuf, recvcnt, rtype_ptr, comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno, myname);
}
