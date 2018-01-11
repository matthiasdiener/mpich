/*
 *  $Id: cartdim_get.c,v 1.10 1997/01/07 01:48:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Cartdim_get - Retrieves Cartesian topology information associated with a 
                  communicator

Input Parameter:
. comm - communicator with cartesian structure (handle) 

Output Parameter:
. ndims - number of dimensions of the cartesian structure (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Cartdim_get ( comm, ndims )
MPI_Comm  comm;
int      *ndims;
{
  int mpi_errno, flag;
  MPIR_TOPOLOGY *topo;
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_CARTDIM_GET";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  /* Check for valid arguments */
  if ( MPIR_TEST_ARG(ndims) )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Set dims */
  if ( ndims != (int *)0 )
    if ( (flag == 1) && (topo->type == MPI_CART) )
      (*ndims) = topo->cart.ndims;
    else
      (*ndims) = MPI_UNDEFINED;

  TR_POP;
  return (MPI_SUCCESS);
}
