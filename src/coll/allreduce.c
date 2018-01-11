/*
 *  $Id: allreduce.c,v 1.19 1995/12/21 22:16:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: allreduce.c,v 1.19 1995/12/21 22:16:31 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Allreduce - Combines values from all processes and distribute the result
                back to all processes

Input Parameters:
. sendbuf - starting address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - operation (handle) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - starting address of receive buffer (choice) 

.N fortran
@*/
int MPI_Allreduce ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) || 
       MPIR_TEST_DATATYPE(comm,datatype) || MPIR_TEST_COUNT(comm,count) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf))
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_ALLREDUCE" );

  /* Test for intercommunicator is done when collops is assigned */  
  return comm->collops->Allreduce(sendbuf, recvbuf, count, datatype, op, comm );
}
