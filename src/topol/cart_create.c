/*
 *  $Id: cart_create.c,v 1.16 1995/07/25 02:43:56 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*ARGSUSED*/

/*@

MPI_Cart_create - Makes a new communicator to which topology information
                  has been attached

Input Parameters:
. comm_old - input communicator (handle) 
. ndims - number of dimensions of cartesian grid (integer) 
. dims - integer array of size ndims specifying the number of processes in 
  each dimension 
. periods - logical array of size ndims specifying whether the grid is 
  periodic (true) or not (false) in each dimension 
. reorder - ranking may be reordered (true ) or not (false ) (logical) 

Output Parameter:
. comm_cart - communicator with new cartesian topology (handle) 

Algorithm:
We ignore the periods and reorder info currently.

@*/
int MPI_Cart_create ( comm_old, ndims, dims, periods, reorder, comm_cart )
MPI_Comm  comm_old;
int       ndims;     
int      *dims;     
int      *periods;
int       reorder;
MPI_Comm *comm_cart;
{

  int range[1][3];
  MPI_Group group_old, group;
  int i, rank, num_ranks = 1;
  int mpi_errno = MPI_SUCCESS;
  int flag, size;
  MPIR_TOPOLOGY *topo;

  /* Check validity of arguments */
  if (MPIR_TEST_COMM(comm_old,comm_old) || MPIR_TEST_ARG(comm_cart) ||
      MPIR_TEST_ARG(periods)  ||
      ((ndims     <  1)             && (mpi_errno = MPI_ERR_DIMS)) ||
      ((dims      == (int *)0)      && (mpi_errno = MPI_ERR_DIMS)))
    return MPIR_ERROR( comm_old, mpi_errno, "Error in MPI_CART_CREATE" );

  /* Check for Intra-communicator */
  MPI_Comm_test_inter ( comm_old, &flag );
  if (flag)
    return MPIR_ERROR(comm_old, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_CART_CREATE");

  /* Determine number of ranks in topology */
  for ( i=0; i<ndims; i++ )
    num_ranks    *= (dims[i]>0)?dims[i]:-dims[i];
  if ( num_ranks < 1 ) {
    (*comm_cart)  = MPI_COMM_NULL;
    return MPIR_ERROR( comm_old, MPI_ERR_TOPOLOGY, 
		      "Error in MPI_CART_CREATE" );
  }

  /* Is the old communicator big enough? */
  MPIR_Comm_size (comm_old, &size);
  if (num_ranks > size) 
	return MPIR_ERROR(comm_old, MPI_ERR_ARG, 
				  "Topology size too big in MPI_CART_CREATE");
	
  /* Make new comm */
  range[0][0] = 0; range[0][1] = num_ranks - 1; range[0][2] = 1;
  MPI_Comm_group ( comm_old, &group_old );
  MPI_Group_range_incl ( group_old, 1, range, &group );
  MPI_Comm_create  ( comm_old, group, comm_cart );
  MPI_Group_free( &group );
  MPI_Group_free( &group_old );

  /* Store topology information in new communicator */
  if ( (*comm_cart) != MPI_COMM_NULL ) {
    topo = (MPIR_TOPOLOGY *) MPIR_SBalloc ( MPIR_topo_els );
    MPIR_SET_COOKIE(&topo->cart,MPIR_CART_TOPOL_COOKIE)
    topo->cart.type         = MPI_CART;
    topo->cart.nnodes       = num_ranks;
    topo->cart.ndims        = ndims;
    topo->cart.dims         = (int *)MALLOC( sizeof(int) * 3 * ndims );
    topo->cart.periods      = topo->cart.dims + ndims;
    topo->cart.position     = topo->cart.periods + ndims;
    for ( i=0; i<ndims; i++ ) {
      topo->cart.dims[i]    = dims[i];
      topo->cart.periods[i] = periods[i];
    }

    /* Compute my position */
    MPIR_Comm_rank ( (*comm_cart), &rank );
    for ( i=0; i < ndims; i++ ) {
      num_ranks = num_ranks / dims[i];
      topo->cart.position[i]  = rank / num_ranks;
      rank   = rank % num_ranks;
    }

    /* cache topology information */
    MPI_Attr_put ( (*comm_cart), MPIR_TOPOLOGY_KEYVAL, (void *)topo );
  }
  return (mpi_errno);
}
