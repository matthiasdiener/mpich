/*
 *  $Id: cart_get.c,v 1.6 1994/08/10 18:52:29 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Cart_get - Retrieves Cartesian topology information associated with a 
               communicator

Input Parameters:
. comm - communicator with cartesian structure (handle) 
. maxdims - length of vectors  dims, periods , and  coords in the calling program (integer) 

Output Parameters:
. dims - number of processes for each cartesian dimension (array of integer) 
. periods - periodicity (true /false ) for each cartesian dimension (array of logical) 
. coords - coordinates of calling process in cartesian structure (array of integer) 

@*/
int MPI_Cart_get ( comm, maxdims, dims, periods, coords )
MPI_Comm comm;
int maxdims;
int *dims, *periods, *coords;
{
  int i, num, flag;
  int *array;
  int errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;

  /* Check for valid arguments */
  if (MPIR_TEST_COMM(comm,comm))
    return MPIR_ERROR( comm, errno, "Error in MPI_CART_GET" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)                 && (errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)    && (errno = MPI_ERR_TOPOLOGY))  )
    return MPIR_ERROR( comm, errno, "Error in MPI_CART_GET" );

  /* Get dims */
  num = topo->cart.ndims;
  array = topo->cart.dims;
  if ( dims != (int *)0 )
    for ( i=0; (i<maxdims) && (i<num); i++ )
      (*dims++) = (*array++);

  /* Get periods */
  num = topo->cart.ndims;
  array = topo->cart.periods;
  if ( periods != (int *)0 )
    for ( i=0; (i<maxdims) && (i<num); i++ )
      (*periods++) = (*array++);

  /* Get coords */
  num = topo->cart.ndims;
  array = topo->cart.position;
  if ( coords != (int *)0 )
    for ( i=0; (i<maxdims) && (i<num); i++ )
      (*coords++) = (*array++);

  return (errno);
}
