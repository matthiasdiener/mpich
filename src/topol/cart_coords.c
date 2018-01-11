/*
 *  $Id: cart_coords.c,v 1.12 1994/12/15 17:33:17 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Cart_coords - Determines process coords in cartesian topology given
                  rank in group

Input Parameters:
. comm - communicator with cartesian structure (handle) 
. rank - rank of a process within group of comm (integer) 
. maxdims - length of vector  coord in the calling program (integer) 

Output Parameter:
. coords - integer array (of size  ndims ) containing the cartesian coordinates of specified process (integer) 

@*/
int MPI_Cart_coords ( comm, rank, maxdims, coords )
MPI_Comm  comm;
int       rank;
int       maxdims;
int      *coords;
{
  int i, *dims, flag;
  int mpi_errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;

  /* Check for valid arguments */
  if ( MPIR_TEST_COMM( comm, comm ) ||
       ((rank                <  0)       && (mpi_errno = MPI_ERR_RANK))      ||
       ((maxdims             <  1)       && (mpi_errno = MPI_ERR_DIMS))      ||
       MPIR_TEST_ARG(coords) )
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_CART_COORDS" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)                 && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)    && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (rank >= topo->cart.nnodes) && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_CART_COORDS" );

  /* Calculate coords */
  for ( i=0; (i < topo->cart.ndims) && (i < maxdims); i++ ) {
    topo->cart.nnodes = topo->cart.nnodes / topo->cart.dims[i];
    coords[i]         = rank / topo->cart.nnodes;
    rank              = rank % topo->cart.nnodes;
  }

  return (mpi_errno);
}
