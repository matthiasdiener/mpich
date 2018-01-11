/*
 *  $Id: cart_rank.c,v 1.17 1995/12/21 22:18:21 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Cart_rank - Determines process rank in communicator given Cartesian
                location

Input Parameters:
. comm - communicator with cartesian structure (handle) 
. coords - integer array (of size  'ndims') specifying the cartesian 
  coordinates of a process 

Output Parameter:
. rank - rank of specified process (integer) 

.N fortran
@*/
int MPI_Cart_rank ( comm, coords, rank )
MPI_Comm comm;
int *coords;
int *rank;
{
  int i, ndims, multiplier = 1;
  int mpi_errno = MPI_SUCCESS;
  int coord, flag;
  MPIR_TOPOLOGY *topo;

  /* Check for valid arguments */
  if (MPIR_TEST_COMM(comm,comm) ||
      ((rank == (int *)0) &&(mpi_errno = MPI_ERR_BUFFER)))
      return MPIR_ERROR( comm, mpi_errno, "Error in MPI_CART_RANK" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)               && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)  && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->cart.ndims < 1)    && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_CART_RANK" );

  /* Compute rank */
  ndims = topo->cart.ndims;
  (*rank) = coords[ndims-1];
  if ( !(topo->cart.periods[ndims-1]) ) {
    if ( ((*rank) >= topo->cart.dims[ndims-1]) ||
         ((*rank) <  0) ) {
      (*rank) = MPI_PROC_NULL;
      return (mpi_errno);
    }
  }
  else {
    if ( (*rank) >= topo->cart.dims[ndims-1] )
      (*rank) = (*rank) % topo->cart.dims[ndims-1];
    else if ( (*rank) <  0 )  {
      (*rank) = ((*rank) % topo->cart.dims[ndims-1]);
      if (*rank) (*rank) = topo->cart.dims[ndims-1] + (*rank);
    }
  }

  for ( i=ndims-2; i >=0; i-- ) {
    coord = coords[i];
    if ( !(topo->cart.periods[i]) ) {
      if ( (coord >= topo->cart.dims[i]) ||
           (coord <  0) ) {
        (*rank) = MPI_PROC_NULL;
        return (mpi_errno);
      }
    }
    else {
      if (coord >= topo->cart.dims[i])
        coord = coord % topo->cart.dims[i];
      else if (coord <  0) {
        coord = coord % topo->cart.dims[i];
        if (coord) coord = topo->cart.dims[i] + coord;
      }
    }
    multiplier *= topo->cart.dims[i+1];
    (*rank) += multiplier * coord;
  }

  return (mpi_errno);
}

