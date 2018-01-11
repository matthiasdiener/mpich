/*
 *  $Id: red_scat.c,v 1.25 1997/01/07 01:47:46 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"
#include "mpiops.h"

/*@

MPI_Reduce_scatter - Combines values and scatters the results

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. recvcounts - integer array specifying the 
number of elements in result distributed to each process.
Array must be identical on all calling processes. 
. datatype - data type of elements of input buffer (handle) 
. op - operation (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - starting address of receive buffer (choice) 

.N fortran

.N collops

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
.N MPI_ERR_OP
.N MPI_ERR_BUFFER_ALIAS
@*/
int MPI_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  int   mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *dtype_ptr;
  static char myname[] = "MPI_REDUCE_SCATTER";
  MPIR_ERROR_DECL;

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

  /* Check for invalid arguments */
  if ( MPIR_TEST_ALIAS(recvbuf,sendbuf) )
    return MPIR_ERROR(comm_ptr, mpi_errno, myname );

  MPIR_ERROR_PUSH(comm_ptr);
  mpi_errno = comm_ptr->collops->Reduce_scatter(sendbuf, recvbuf, recvcnts, 
						dtype_ptr, op, comm_ptr );
  MPIR_ERROR_POP(comm_ptr);
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
