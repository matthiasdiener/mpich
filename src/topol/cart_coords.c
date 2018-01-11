/*
 *  $Id: cart_coords.c,v 1.2 1998/04/29 14:28:29 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Cart_coords - Determines process coords in cartesian topology given
                  rank in group

Input Parameters:
+ comm - communicator with cartesian structure (handle) 
. rank - rank of a process within group of 'comm' (integer) 
- maxdims - length of vector 'coords' in the calling program (integer) 

Output Parameter:
. coords - integer array (of size 'ndims') containing the cartesian coordinates of specified process (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_RANK
.N MPI_ERR_DIMS
.N MPI_ERR_ARG

@*/
int MPI_Cart_coords ( comm, rank, maxdims, coords )
MPI_Comm  comm;
int       rank;
int       maxdims;
int      *coords;
{
  int i, flag;
  int mpi_errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;
  int nnodes;
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_CART_COORDS";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  /* Check for valid arguments */
  if ( ((rank                <  0)       && (mpi_errno = MPI_ERR_RANK))      ||
       ((maxdims             <  1)       && (mpi_errno = MPI_ERR_DIMS))      ||
       MPIR_TEST_ARG(coords) )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)                 && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)    && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (rank >= topo->cart.nnodes) && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Calculate coords */
  nnodes = topo->cart.nnodes;
  for ( i=0; (i < topo->cart.ndims) && (i < maxdims); i++ ) {
    nnodes    = nnodes / topo->cart.dims[i];
    coords[i] = rank / nnodes;
    rank      = rank % nnodes;
  }

  TR_POP;
  return (mpi_errno);
}
