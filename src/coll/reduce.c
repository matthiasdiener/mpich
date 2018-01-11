/*
 *  $Id: reduce.c,v 1.34 1995/12/21 22:17:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: reduce.c,v 1.34 1995/12/21 22:17:26 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

/*@

MPI_Reduce - Reduces values on all processes to a single value

Input Parameters:
. sendbuf - address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - reduce operation (handle) 
. root - rank of root process (integer) 
. comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, 
significant only at 'root') 

Algorithm:
This implementation currently uses a simple tree algorithm.

.N fortran
@*/
int MPI_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
int               root;
MPI_Comm          comm;
{
  MPI_Status status;
  int        size, rank;
  int        mask, relrank, source, lroot;
  int        mpi_errno = MPI_SUCCESS;
  MPI_User_function *uop;
  int        flag;
  MPI_Aint   extent;
  void       *buffer;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_OP(comm,op) ||
       MPIR_TEST_ALIAS(sendbuf,recvbuf) )
    return MPIR_ERROR(comm, mpi_errno, "Error in MPI_REDUCE" );

  return comm->collops->Reduce(sendbuf, recvbuf, count, datatype, op, root, comm );
}
