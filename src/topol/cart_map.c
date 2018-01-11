/*
 *  $Id: cart_map.c,v 1.7 1995/12/21 22:18:17 gropp Exp $
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
  int rank;
  int mpi_errno = MPI_SUCCESS;

  /* Check for valid arguments */
  if (MPIR_TEST_COMM(comm_old,comm_old) ||
      ((ndims < 1)                 && (mpi_errno = MPI_ERR_DIMS))      ||
      ((newrank == (int *)0)       && (mpi_errno = MPI_ERR_ARG))      ||
      ((dims == (int *)0)          && (mpi_errno = MPI_ERR_ARG))       )
    return MPIR_ERROR( comm_old, mpi_errno, "Error in MPI_CART_MAP" );
  
  /* Determine number of processes needed for topology */
  for ( i=0; i<ndims; i++ )
    nranks *= dims[i];

  /* Am I in this range? */
  MPIR_Comm_rank ( comm_old, &rank );
  if ( rank < nranks )
    (*newrank) = rank;
  else
    (*newrank) = MPI_UNDEFINED;

  return (mpi_errno);
}
