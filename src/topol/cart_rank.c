/*
 *  $Id: cart_rank.c,v 1.13 1994/08/10 18:52:29 doss Exp $
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
. coords - integer array (of size  ndims ) specifying the cartesian coordinates of a process 
Output Parameter:
. rank - rank of specified process (integer) 

@*/
int MPI_Cart_rank ( comm, coords, rank )
MPI_Comm comm;
int *coords;
int *rank;
{
  int i, ndims, multiplier = 1;
  int errno = MPI_SUCCESS;
  int coord, flag;
  MPIR_TOPOLOGY *topo;

  /* Check for valid arguments */
  if (MPIR_TEST_COMM(comm,comm) ||
      ((rank == (int *)0) &&(errno = MPI_ERR_BUFFER))) 
      return MPIR_ERROR( comm, errno, "Error in MPI_CART_RANK" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)               && (errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)  && (errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->cart.ndims < 1)    && (errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm, errno, "Error in MPI_CART_RANK" );

  /* Compute rank */
  ndims = topo->cart.ndims;
  (*rank) = coords[ndims-1];
  if ( !(topo->cart.periods[ndims-1]) ) {
    if ( ((*rank) == MPI_PROC_NULL)                          ||
         ((*rank) >= topo->cart.dims[ndims-1]) ||
         ((*rank) <  0) ) {
      (*rank) = MPI_PROC_NULL;
      return (errno);
    }
  }
  else {
    if ( (*rank) == MPI_PROC_NULL ) {
        (*rank) = MPI_PROC_NULL;
        return (errno);
    }
    if ( ( (*rank) >= topo->cart.dims[ndims-1] ) ||
         ( (*rank) <  0 ) ) {
      (*rank) = (*rank) % topo->cart.dims[ndims-1];
    }
  }
  
  for ( i=ndims-2; i >=0; i-- ) {
    coord = coords[i];
    if ( coord == MPI_PROC_NULL ) {
      (*rank) = MPI_PROC_NULL;
      return (errno);
    }
    if ( !(topo->cart.periods[i]) ) {
      if ( (coord >= topo->cart.dims[i]) ||
           (coord <  0) ) {
        (*rank) = MPI_PROC_NULL;
        return (errno);
      }
    }
    else {
      if ( (coord >= topo->cart.dims[i]) ||
           (coord <  0) )
        coord = coord % topo->cart.dims[i];
    }
    multiplier *= topo->cart.dims[i+1];
    (*rank) += multiplier * coord;
  }
  
  return (errno);
}
