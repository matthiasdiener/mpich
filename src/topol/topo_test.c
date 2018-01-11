/*
 *  $Id: topo_test.c,v 1.8 1995/12/21 22:19:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Topo_test - Determines the type of topology (if any) associated with a 
                communicator

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. top_type - topology type of communicator 'comm' (choice) 

.N fortran
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
