/*
 *  $Id: graph_get.c,v 1.7 1995/12/21 22:18:49 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Graph_get - Retrieves graph topology information associated with a 
                communicator

Input Parameters:
. comm - communicator with graph structure (handle) 
. maxindex - length of vector 'index' in the calling program  (integer) 
. maxedges - length of vector 'edges' in the calling program  (integer) 

Output Parameter:
. index - array of integers containing the graph structure (for details see the definition of 'MPI_GRAPH_CREATE') 
. edges - array of integers containing the graph structure 

.N fortran
@*/
int MPI_Graph_get ( comm, maxindex, maxedges, index, edges )
MPI_Comm comm;
int maxindex, maxedges;
int *index, *edges;
{
  int i, num, flag;
  int *array;
  int mpi_errno = MPI_SUCCESS;
  MPIR_TOPOLOGY *topo;

  if (MPIR_TEST_COMM(comm,comm))
      return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_GET" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );
  if ( ( (flag != 1)               && (mpi_errno = MPI_ERR_TOPOLOGY) ) ||
       ( (topo->type != MPI_GRAPH) && (mpi_errno = MPI_ERR_TOPOLOGY) )  )
      return MPIR_ERROR( comm, mpi_errno, "Error in MPI_GRAPH_GET" );

  /* Get index */
  num = topo->graph.nnodes;
  array = topo->graph.index;
  if ( index != (int *)0 )
    for ( i=0; (i<maxindex) && (i<num); i++ )
      (*index++) = (*array++);

  /* Get edges */
  num = topo->graph.nedges;
  array = topo->graph.edges;
  if ( edges != (int *)0 )
    for ( i=0; (i<maxedges) && (i<num); i++ )
      (*edges++) = (*array++);

  return (mpi_errno);
}
