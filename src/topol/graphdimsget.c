/*
 *  $Id: graphdimsget.c,v 1.5 1997/01/07 01:48:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Graphdims_get - Retrieves graph topology information associated with a 
                    communicator

Input Parameters:
. comm - communicator for group with graph structure (handle) 

Output Parameter:
. nnodes - number of nodes in graph (integer) 
. nedges - number of edges in graph (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TOPOLOGY
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Graphdims_get ( comm, nnodes, nedges )
MPI_Comm  comm;
int              *nnodes;
int              *nedges;
{
  int mpi_errno, flag;
  MPIR_TOPOLOGY *topo;
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_GRAPHDIMS_GET";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if ( MPIR_TEST_ARG(nnodes) || MPIR_TEST_ARG(nedges) )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Get topology information from the communicator */
  mpi_errno = 
      MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  if (mpi_errno) {
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );
  }

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

  TR_POP;
  return (MPI_SUCCESS);
}
