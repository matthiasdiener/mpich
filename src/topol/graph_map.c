/*
 *  $Id: graph_map.c,v 1.10 1997/01/07 01:48:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Graph_map - Maps process to graph topology information

Input Parameters:
. comm - input communicator (handle) 
. nnodes - number of graph nodes (integer) 
. index - integer array specifying the graph structure, see 'MPI_GRAPH_CREATE' 
. edges - integer array specifying the graph structure 

Output Parameter:
. newrank - reordered rank of the calling process; 'MPI_UNDEFINED' if the 
calling process does not belong to graph (integer) 
 
.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Graph_map ( comm_old, nnodes, index, edges, newrank )
MPI_Comm comm_old;
int      nnodes;
int     *index;
int     *edges;
int     *newrank;
{
  int rank, size;
  int mpi_errno = MPI_SUCCESS;
  struct MPIR_COMMUNICATOR *comm_old_ptr;
  static char myname[] = "MPI_GRAPH_MAP";

  TR_PUSH(myname);
  comm_old_ptr = MPIR_GET_COMM_PTR(comm_old);
  MPIR_TEST_MPI_COMM(comm_old,comm_old_ptr,comm_old_ptr,myname);

  if (((nnodes   <  1)             && (mpi_errno = MPI_ERR_ARG))  ||
      MPIR_TEST_ARG(newrank) || MPIR_TEST_ARG(index) ||
      MPIR_TEST_ARG(edges))
    return MPIR_ERROR( comm_old_ptr, mpi_errno, myname );
  
  /* Test that the communicator is large enough */
  MPIR_Comm_size( comm_old_ptr, &size );
  if (size < nnodes) {
      return MPIR_ERROR( comm_old_ptr, MPI_ERR_ARG, myname );
  }

  /* Am I in this topology? */
  MPIR_Comm_rank ( comm_old_ptr, &rank );
  if ( rank < nnodes )
    (*newrank) = rank;
  else
    (*newrank) = MPI_UNDEFINED;

  TR_POP;
  return (mpi_errno);
}
