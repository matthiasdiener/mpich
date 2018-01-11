/*
 *  $Id: topo_test.c,v 1.11 1997/01/07 01:48:01 gropp Exp $
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
  struct MPIR_COMMUNICATOR *comm_ptr;
  static char myname[] = "MPI_TOPO_TEST";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if ( MPIR_TEST_ARG(top_type) )
    return MPIR_ERROR( comm_ptr, mpi_errno, myname );
  
  /* Set the top_type */
  /* Get topology information from the communicator */
  mpi_errno = MPI_Attr_get ( comm, MPIR_TOPOLOGY_KEYVAL, (void **)&topo, 
			     &flag );
  if (mpi_errno) return MPIR_ERROR( comm_ptr, mpi_errno, myname );

  /* Check for topology information */
  if ( flag == 1 )
    (*top_type) = topo->type;
  else
    (*top_type) = MPI_UNDEFINED;

  TR_POP;
  return (MPI_SUCCESS);
}
