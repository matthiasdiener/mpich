/*
 *  $Id: cart_map.c,v 1.10 1997/01/07 01:48:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Cart_map - Maps process to Cartesian topology information 

Input Parameters:
. comm - input communicator (handle) 
. ndims - number of dimensions of cartesian structure (integer) 
. dims - integer array of size 'ndims' specifying the number of processes in each coordinate direction 
. periods - logical array of size 'ndims' specifying the periodicity specification in each coordinate direction 

Output Parameter:
. newrank - reordered rank of the calling process; 'MPI_UNDEFINED' if calling process does not belong to grid (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_DIMS
.N MPI_ERR_ARG
@*/
int MPI_Cart_map ( comm_old, ndims, dims, periods, newrank )
MPI_Comm comm_old;
int      ndims;
int     *dims;
int     *periods;
int     *newrank;
{
  int i;
  int nranks = 1;
  int rank, size;
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_old_ptr;
  static char myname[] = "MPI_CART_MAP";

  TR_PUSH(myname);
  comm_old_ptr = MPIR_GET_COMM_PTR(comm_old);
  MPIR_TEST_MPI_COMM(comm_old,comm_old_ptr,comm_old_ptr,myname);

  /* Check for valid arguments */
  if (((ndims < 1)                 && (mpi_errno = MPI_ERR_DIMS))      ||
      ((newrank == (int *)0)       && (mpi_errno = MPI_ERR_ARG))      ||
      ((dims == (int *)0)          && (mpi_errno = MPI_ERR_ARG))       )
    return MPIR_ERROR( comm_old_ptr, mpi_errno, myname );
  
  /* Determine number of processes needed for topology */
  for ( i=0; i<ndims; i++ )
    nranks *= dims[i];

  /* Test that the communicator is large enough */
  MPIR_Comm_size( comm_old_ptr, &size );
  if (size < nranks) {
      return MPIR_ERROR( comm_old_ptr, MPI_ERR_ARG, myname );
  }

  /* Am I in this range? */
  MPIR_Comm_rank ( comm_old_ptr, &rank );
  if ( rank < nranks )
    (*newrank) = rank;
  else
    (*newrank) = MPI_UNDEFINED;

  TR_POP;
  return (mpi_errno);
}
