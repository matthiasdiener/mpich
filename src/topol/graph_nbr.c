/*
 *  $Id: graph_nbr.c,v 1.14 1994/12/15 17:34:40 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifndef MPIR_MIN
#define MPIR_MIN(a,b) ((a)>(b)?(b):(a))
#endif

/*@

MPI_Graph_neighbors - Returns the neighbors of a node associated 
                      with a graph topology

Input Parameters:
. comm - communicator with graph topology (handle) 
. rank - rank of process in group of comm (integer) 
. maxneighbors - size of array neighbors (integer) 

Output Parameters:
. neighbors - ranks of processes that are neighbors to specified process (array of integer) 

@*/
int MPI_Graph_neighbors ( comm, rank, maxneighbors, neighbors )
MPI_Comm  comm;
int       rank;
int      maxneighbors;
int      *neighbors;
{
  int i, begin, end, flag;
  int mpi_errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;

  if (MPIR_TEST_COMM(comm,comm)               ||
      ((rank <  0) && (mpi_errno = MPI_ERR_RANK)) ||
      MPIR_TEST_ARG(neighbors))
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_NEIGHBORS" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)                  && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_GRAPH)    && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (rank >= topo->graph.nnodes) && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_NEIGHBORS" );

  /* Get neighbors */
  if ( rank == 0 ) begin = 0;
  else             begin = topo->graph.index[rank-1];
  end = topo->graph.index[rank];
  for ( i=begin; i<end; i++ )
    neighbors[i-begin] = topo->graph.edges[i];

  return (mpi_errno);
}
