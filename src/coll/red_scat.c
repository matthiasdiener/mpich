/*
 *  $Id: red_scat.c,v 1.22 1995/12/21 22:17:21 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: red_scat.c,v 1.22 1995/12/21 22:17:21 gropp Exp $";
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

.N fortran
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

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(recvbuf,sendbuf) || MPIR_TEST_DATATYPE(comm,datatype))
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE_SCATTER" );

  return comm->collops->Reduce_scatter(sendbuf, recvbuf, recvcnts, datatype, op, comm );
}
