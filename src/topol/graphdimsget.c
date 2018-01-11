/*
 *  $Id: graphdimsget.c,v 1.1 1995/04/23 18:11:33 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Graphdims_get - Retrieves graph topology information associated with a 
                    communicator

Input Parameters:
. comm - communicator for group with graph structure (handle) 

Output Parameter:
. nnodes - number of nodes in graph (integer) 
. nedges - number of edges in graph (integer) 

@*/
int MPI_Graphdims_get ( comm, nnodes, nedges )
MPI_Comm  comm;
int              *nnodes;
int              *nedges;
{
  int mpi_errno, flag;
  MPIR_TOPOLOGY *topo;

  if ( MPIR_TEST_COMM(comm,comm) )
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
		       "Error in MPI_GRAPHDIMS_GET" );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Set nnodes */
  if ( nnodes != (int *)0 )
    if ( (flag == 1) && (topo->type == MPI_GRAPH) )
      (*nnodes) = topo->graph.nnodes;
    else
      (*nnodes) = MPI_UNDEFINED;

  /* Set nedges */
  if ( nedges != (int *)0 ) 
    if ( (flag == 1) && (topo->type == MPI_GRAPH) )
      (*nedges) = topo->graph.nedges;
    else
      (*nedges) = MPI_UNDEFINED;

  return (MPI_SUCCESS);
}
