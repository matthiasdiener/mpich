/*
 *  $Id: graphnbrcnt.c,v 1.3 1996/04/12 15:56:36 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Graph_neighbors_count - Returns the number of neighbors of a node
                            associated with a graph topology

Input Parameters:
. comm - communicator with graph topology (handle) 
. rank - rank of process in group of 'comm' (integer) 

Output Parameter:
. nneighbors - number of neighbors of specified process (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_COMM
.N MPI_ERR_ARG
.N MPI_ERR_RANK
@*/
int MPI_Graph_neighbors_count ( comm, rank, nneighbors )
MPI_Comm  comm;
int       rank;
int      *nneighbors;
{
  int mpi_errno = MPI_SUCCESS;
  int flag;
  MPIR_TOPOLOGY *topo;

  if (MPIR_TEST_COMM(comm,comm)                ||
      ((rank <  0) && (mpi_errno = MPI_ERR_RANK))  ||
      MPIR_TEST_ARG(nneighbors))
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_NEIGHBORS_COUNT" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for valid topology */
  if ( ( (flag != 1)                  && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (topo->type != MPI_GRAPH)    && (mpi_errno = MPI_ERR_TOPOLOGY))  ||
       ( (rank >= topo->graph.nnodes) && (mpi_errno = MPI_ERR_RANK))      )
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_NEIGHBORS_COUNT" );

  /* Get nneighbors */
  if ( rank == 0 ) 
    (*nneighbors) = topo->graph.index[rank];
  else
    (*nneighbors) = topo->graph.index[rank] - topo->graph.index[rank-1];
  
  return (mpi_errno);
}
