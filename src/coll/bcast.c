/*
 *  $Id: bcast.c,v 1.27 1995/12/21 22:16:51 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: bcast.c,v 1.27 1995/12/21 22:16:51 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "coll.h"

/*@

MPI_Bcast - Broadcasts a message from the process with rank "root" to
            all other processes of the group. 

Input/output Parameters:
. buffer - starting address of buffer (choice) 
. count - number of entries in buffer (integer) 
. datatype - data type of buffer (handle) 
. root - rank of broadcast root (integer) 
. comm - communicator (handle) 

Algorithm:  
If the underlying device does not take responsibility, this function
uses a tree-like algorithm to broadcast the message to blocks of
processes.  A linear algorithm is then used to broadcast the message
from the first process in a block to all other processes.
'MPIR_BCAST_BLOCK_SIZE' determines the size of blocks.  If this is set
to 1, then this function is equivalent to using a pure tree algorithm.
If it is set to the size of the group or greater, it is a pure linear
algorithm.  The value should be adjusted to determine the most
efficient value on different machines.

.N fortran
@*/
int MPI_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
MPI_Datatype      datatype;
int               root;
MPI_Comm          comm;
{
  MPI_Status status;
  int        rank, size, src, dst;
  int        n, N, surfeit, N2_prev, N2_next, N_rank;
  int        participants, my_block, my_offset;
  int        mpi_errno = MPI_SUCCESS;
  int        bsize;
  int        int_n;
  int        flag;

  /* Check for invalid arguments */
  if ( MPIR_TEST_COMM(comm,comm) ||
   ( (root            <  0)          && (mpi_errno = MPI_ERR_ROOT) )) 
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_BCAST" );

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  return comm->collops->Bcast(buffer, count, datatype, root, comm);
}
