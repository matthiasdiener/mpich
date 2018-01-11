/*
 *  $Id: cart_rank.c,v 1.20 1997/01/07 01:48:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

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

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_RANK
.N MPI_ERR_ARG
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
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_CART_RANK";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  /* Check for valid arguments */
  if (((rank == (int *)0) &&(mpi_errno = MPI_ERR_ARG)))
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)               && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)  && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->cart.ndims < 1)    && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

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

  TR_POP;
  return (mpi_errno);
}

