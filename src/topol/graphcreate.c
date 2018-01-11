/*
 *  $Id: graphcreate.c,v 1.1 1995/04/23 18:11:31 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/*@

MPI_Graph_create - Makes a new communicator to which topology information
                 has been attached

Input Parameters:
. comm_old - input communicator without topology (handle) 
. nnodes - number of nodes in graph (integer) 
. index - array of integers describing node degrees (see below) 
. edges - array of integers describing graph edges (see below) 
. reorder - ranking may be reordered (true ) or not (false ) (logical) 

Output Parameter:
. comm_graph - communicator with graph topology added (handle) 

Algorithm:
We ignore the periods and reorder info currently.

@*/
int MPI_Graph_create ( comm_old, nnodes, index, edges, reorder, comm_graph )
MPI_Comm  comm_old;
int       nnodes;
int      *index;
int      *edges;
int       reorder;
MPI_Comm *comm_graph;
{
  int range[1][3];
  MPI_Group group_old, group;
  int i, rank, num_ranks = 1;
  int mpi_errno = MPI_SUCCESS;
  int flag, size;
  MPIR_TOPOLOGY *topo;

  /* Check validity of arguments */
  if (MPIR_TEST_COMM(comm_old,comm_old) || 
      MPIR_TEST_ARG(comm_graph) || MPIR_TEST_ARG(index) || 
      MPIR_TEST_ARG(edges) || 
      ((nnodes     <  1)             && (mpi_errno = MPI_ERR_ARG))   )
    return MPIR_ERROR( comm_old, mpi_errno, "Error in MPI_GRAPH_CREATE" );

  /* Check for Intra-communicator */
  MPI_Comm_test_inter ( comm_old, &flag );
  if (flag)
    return MPIR_ERROR(comm_old, MPI_ERR_COMM,
                      "Inter-communicator invalid in MPI_GRAPH_CREATE");
  
  /* Determine number of ranks in topology */
  num_ranks = nnodes;
  if ( num_ranks < 1 ) {
    (*comm_graph)  = MPI_COMM_NULL;
    return MPIR_ERROR( comm_old, MPI_ERR_TOPOLOGY, 
		       "Error in MPI_GRAPH_CREATE" );
  }

  /* Is the old communicator big enough? */
  MPI_Comm_size (comm_old, &size);
  if (num_ranks > size) 
	return MPIR_ERROR(comm_old, MPI_ERR_ARG, 
			         "Topology size too big in MPI_GRAPH_CREATE");

  /* Make new communicator */
  range[0][0] = 0; range[0][1] = num_ranks - 1; range[0][2] = 1;
  MPI_Comm_group ( comm_old, &group_old );
  MPI_Group_range_incl ( group_old, 1, range, &group );
  MPI_Comm_create  ( comm_old, group, comm_graph );
  MPI_Group_free( &group_old );
  MPI_Group_free( &group );

  /* Store topology information in new communicator */
  if ( (*comm_graph) != MPI_COMM_NULL ) {
    topo = (MPIR_TOPOLOGY *) MPIR_SBalloc ( MPIR_topo_els );
    MPIR_SET_COOKIE(&topo->graph,MPIR_GRAPH_TOPOL_COOKIE)
    topo->graph.type       = MPI_GRAPH;
    topo->graph.nnodes     = nnodes;
    topo->graph.nedges     = index[nnodes-1];
    topo->graph.index      = (int *)MALLOC(sizeof(int)*(nnodes+index[nnodes-1]));
    topo->graph.edges      = topo->graph.index + nnodes;
    for ( i=0; i<nnodes; i++ )
      topo->graph.index[i] = index[i];
    for ( i=0; i<index[nnodes-1]; i++ )
      topo->graph.edges[i] = edges[i];

    /* cache topology information */
    MPI_Attr_put ( (*comm_graph), MPIR_TOPOLOGY_KEYVAL, (void *)topo );
  }
  return (mpi_errno);
}