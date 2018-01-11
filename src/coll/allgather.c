/*
 *  $Id: allgather.c,v 1.3 1998/04/28 18:50:38 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Allgather - Gathers data from all tasks and distribute it to all 

Input Parameters:
+ sendbuf - starting address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - data type of send buffer elements (handle) 
. recvcount - number of elements received from any process (integer) 
. recvtype - data type of receive buffer elements (handle) 
- comm - communicator (handle) 

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
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
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
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *stype_ptr, *rtype_ptr;
  MPIR_ERROR_DECL;
  static char myname[] = "MPI_ALLGATHER";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr, myname);

  stype_ptr = MPIR_GET_DTYPE_PTR(sendtype);
  MPIR_TEST_DTYPE(sendtype,stype_ptr,comm_ptr, myname );

  rtype_ptr = MPIR_GET_DTYPE_PTR(recvtype);
  MPIR_TEST_DTYPE(recvtype,rtype_ptr,comm_ptr, myname );

  /*** Check for mismatched send/recieve types - Debbie Swider 11/20/97 ***/
  if (sendtype != recvtype)
    return MPIR_ERROR( comm_ptr, MPI_ERR_TYPE, myname ); 
 
  /* Check for invalid arguments */
  if ( MPIR_TEST_COUNT(comm,sendcount) ||
       MPIR_TEST_COUNT(comm,recvcount) ) 
      return MPIR_ERROR( comm_ptr, mpi_errno, myname ); 

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Allgather( sendbuf, sendcount, stype_ptr,
					    recvbuf, recvcount, rtype_ptr, 
					    comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}


