/*
 *  $Id: cart_get.c,v 1.3 1998/04/29 14:28:32 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Cart_get - Retrieves Cartesian topology information associated with a 
               communicator

Input Parameters:
+ comm - communicator with cartesian structure (handle) 
- maxdims - length of vectors  'dims', 'periods', and 'coords'
in the calling program (integer) 

Output Parameters:
+ dims - number of processes for each cartesian dimension (array of integer) 
. periods - periodicity (true/false) for each cartesian dimension 
(array of logical) 
- coords - coordinates of calling process in cartesian structure 
(array of integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Cart_get ( comm, maxdims, dims, periods, coords )
MPI_Comm comm;
int maxdims;
int *dims, *periods, *coords;
{
  int i, num, flag;
  int *array;
  int mpi_errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_CART_GET";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  /* Check for valid arguments */

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /*** Check for negative dimension - Debbie Swider 11/19/97 ***/
  if ( (maxdims < 0) && (mpi_errno = MPI_ERR_DIMS) ) 
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Check for valid topology */
  if ( ( (flag != 1)                 && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_CART)    && (mpi_errno = MPI_ERR_TOPOLOGY)) )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

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

  TR_POP;
  return (mpi_errno);
}
