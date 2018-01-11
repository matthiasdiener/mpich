/*
 *  $Id: topo_util.c,v 1.3 1995/05/10 15:31:59 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/* 
   Keyval for topologies.
 */
int MPIR_TOPOLOGY_KEYVAL = MPI_KEYVAL_INVALID;

/*
  MPIR_Topology_copy_fn - copies topology information.
 */
int MPIR_Topology_copy_fn(old_comm, keyval, extra, attr_in, attr_out, flag)
MPI_Comm *old_comm;
int      *keyval;
void     *extra;
void     *attr_in, *attr_out;
int      *flag;
{
  MPIR_TOPOLOGY *old_topo = (MPIR_TOPOLOGY *) attr_in;
  MPIR_TOPOLOGY *new_topo = (MPIR_TOPOLOGY *) MPIR_SBalloc ( MPIR_topo_els );

  /* Copy topology info */
  new_topo->type = old_topo->type;
  if (old_topo->type == MPI_CART) {
    int i, ndims;
    MPIR_SET_COOKIE(&new_topo->cart,MPIR_CART_TOPOL_COOKIE)
    new_topo->cart.nnodes        = old_topo->cart.nnodes; 
    new_topo->cart.ndims = ndims = old_topo->cart.ndims;
    new_topo->cart.dims          = (int *)MALLOC( sizeof(int) * 3 * ndims );
    new_topo->cart.periods       = new_topo->cart.dims + ndims;
    new_topo->cart.position      = new_topo->cart.periods + ndims;
    for ( i=0; i<ndims; i++ ) {
      new_topo->cart.dims[i]     = old_topo->cart.dims[i];
      new_topo->cart.periods[i]  = old_topo->cart.periods[i];
    }
    for ( i=0; i < ndims; i++ ) 
      new_topo->cart.position[i] = old_topo->cart.position[i];
  }
  else if (old_topo->type == MPI_GRAPH) {
    int  i, nnodes;
    int *index;
    MPIR_SET_COOKIE(&new_topo->graph,MPIR_GRAPH_TOPOL_COOKIE)
    new_topo->graph.nnodes = nnodes = old_topo->graph.nnodes;
    new_topo->graph.nedges        = old_topo->graph.nedges;
    index = old_topo->graph.index;
    new_topo->graph.index         = 
      (int *)MALLOC(sizeof(int) * (nnodes + index[nnodes-1]) );
    new_topo->graph.edges         = new_topo->graph.index + nnodes;
    for ( i=0; i<nnodes; i++ )
      new_topo->graph.index[i]    = old_topo->graph.index[i];
    for ( i=0; i<index[nnodes-1]; i++ )
      new_topo->graph.edges[i]    = old_topo->graph.edges[i];
  }

  /* Set attr_out and return a "1" to indicate information was copied */
  (*(void **)attr_out) = (void *) new_topo;
  (*flag)     = 1;
  return (MPI_SUCCESS);
}


/*
  MPIR_Topology_delete_fn - deletes topology information.
 */
int MPIR_Topology_delete_fn(comm, keyval, attr_val, extra)
MPI_Comm *comm;
int      *keyval;
void     *attr_val, *extra;
{
  MPIR_TOPOLOGY *topo = (MPIR_TOPOLOGY *)attr_val;

  /* Free topology specific data */
  if ( topo->type == MPI_CART ) 
	  FREE( topo->cart.dims );
  else if ( topo->type == MPI_GRAPH )
	  FREE( topo->graph.index );
  
  /* Free topology structure */
  MPIR_SBfree ( MPIR_topo_els, topo );

  return (MPI_SUCCESS);
}


/*
MPIR_Topology_init - Initializes topology code.
 */
void MPIR_Topology_init()
{
  MPI_Keyval_create ( MPIR_Topology_copy_fn, 
                      MPIR_Topology_delete_fn,
                      &MPIR_TOPOLOGY_KEYVAL,
                      (void *)0);
}


/*
MPIR_Topology_finalize - Un-initializes topology code.
 */
void MPIR_Topology_finalize()
{
  MPI_Keyval_free ( &MPIR_TOPOLOGY_KEYVAL );
}
