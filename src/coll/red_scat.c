/*
 *  $Id: red_scat.c,v 1.20 1995/05/16 18:09:43 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: red_scat.c,v 1.20 1995/05/16 18:09:43 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

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
@*/
int MPI_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  int   rank, size, i, count=0;
  MPI_Aint extent;
  int  *displs;
  void *buffer;
  int   mpi_errno = MPI_SUCCESS;
  int   flag;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(recvbuf,sendbuf) || MPIR_TEST_DATATYPE(comm,datatype))
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE_SCATTER" );

  /* Check for intra-communicator */
  MPI_Comm_test_inter ( comm, &flag );
  if (flag) 
    return MPIR_ERROR(comm, MPI_ERR_COMM,
			  "Inter-communicator invalid in MPI_REDUCE_SCATTER");

  /* Determine the "count" of items to reduce and set the displacements*/
  MPI_Type_extent (datatype, &extent);
  MPI_Comm_size   (comm, &size);
  MPI_Comm_rank   (comm, &rank);

  /* Allocate the displacements and initialize them */
  displs = (int *)MALLOC(size*sizeof(int));
  if (!displs) 
      return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_REDUCE_SCATTER" );
  for (i=0;i<size;i++) {
    displs[i] = count;
    count += recvcnts[i];
  }

  /* Allocate a temporary buffer */
  if (count == 0) {
      FREE( displs );
      return MPI_SUCCESS;
      }

  buffer = (void *)MALLOC(extent*count);
  if (!buffer) 
      return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_REDUCE_SCATTER" );

  /* Reduce to 0, then scatter */
  mpi_errno = MPI_Reduce   ( sendbuf, buffer, count, datatype, op, 0, comm);
  if (mpi_errno) return mpi_errno;
  mpi_errno = MPI_Scatterv ( buffer, recvcnts, displs, datatype, recvbuf, 
			     recvcnts[rank], datatype, 0, comm );
  
  /* Free the temporary buffers */
  FREE(buffer); FREE(displs);
  return (mpi_errno);
}
