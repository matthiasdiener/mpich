/*
 *  $Id: graph_map.c,v 1.4 1994/07/13 15:54:58 lusk Exp $
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
. index - integer array specifying the graph structure, see  MPI_GRAPH_CREATE 
. edges - integer array specifying the graph structure 

Output Parameter:
. newrank - reordered rank of the calling process; MPI_UNDEFINED if the 
calling process does not belong to graph (integer) 
 

@*/
int MPI_Graph_map ( comm_old, nnodes, index, edges, newrank )
MPI_Comm comm_old;
int      nnodes;
int     *index;
int     *edges;
int     *newrank;
{
  int i;
  int nranks;
  int rank;
  int errno = MPI_SUCCESS;

  if (MPIR_TEST_COMM(comm_old,comm_old) ||
      ((nnodes   <  1)             && (errno = MPI_ERR_ARG))  ||
      MPIR_TEST_ARG(newrank) || MPIR_TEST_ARG(index) ||
      MPIR_TEST_ARG(edges))
    return MPIR_ERROR( comm_old, errno, "Error in MPI_GRAPH_MAP" );
  
  /* Am I in this topology? */
  MPI_Comm_rank ( comm_old, &rank );
  if ( rank < nnodes )
    (*newrank) = rank;
  else
    (*newrank) = MPI_UNDEFINED;

  return (errno);
}
