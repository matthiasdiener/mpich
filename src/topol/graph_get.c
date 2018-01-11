/*
 *  $Id: graph_get.c,v 1.2 1998/04/29 14:28:43 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Graph_get - Retrieves graph topology information associated with a 
                communicator

Input Parameters:
+ comm - communicator with graph structure (handle) 
. maxindex - length of vector 'index' in the calling program  (integer) 
- maxedges - length of vector 'edges' in the calling program  (integer) 

Output Parameter:
+ index - array of integers containing the graph structure (for details see the definition of 'MPI_GRAPH_CREATE') 
- edges - array of integers containing the graph structure 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_COMM
.N MPI_ERR_ARG
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
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_GRAPH_GET";

  TR_PUSH(myname);
  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if (MPIR_TEST_ARG(index) || MPIR_TEST_ARG(edges) )
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );
  if ( ( (flag != 1)               && (mpi_errno = MPI_ERR_TOPOLOGY) ) ||
       ( (topo->type != MPI_GRAPH) && (mpi_errno = MPI_ERR_TOPOLOGY) )  )
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );

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

  TR_POP;
  return (mpi_errno);
}
