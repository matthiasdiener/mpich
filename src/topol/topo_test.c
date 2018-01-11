/*
 *  $Id: topo_test.c,v 1.9 1996/04/12 15:56:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpitopo.h"

/*@

MPI_Topo_test - Determines the type of topology (if any) associated with a 
                communicator

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. top_type - topology type of communicator 'comm' (choice).

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG

.seealso: MPI_Graph_create, MPI_Cart_create
@*/
int MPI_Topo_test ( comm, top_type )
MPI_Comm  comm;
int      *top_type; 
{
  int mpi_errno, flag;
  MPIR_TOPOLOGY *topo;

  if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_ARG(top_type) )
    return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, "Error in MPI_TOPO_TEST" );
  
  /* Set the top_type */
  /* Get topology information from the communicator */
  MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, &flag );

  /* Check for topology information */
  if ( flag == 1 )
    (*top_type) = topo->type;
  else
    (*top_type) = MPI_UNDEFINED;

  return (MPI_SUCCESS);
}
