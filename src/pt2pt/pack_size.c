/*
 *  $Id: pack_size.c,v 1.7 1995/05/09 18:58:56 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: pack_size.c,v 1.7 1995/05/09 18:58:56 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Pack_size - Returns the upper bound on the amount of space needed to
                    pack a message

Input Parameters:
. incount - count argument to packing call (integer) 
. datatype - datatype argument to packing call (handle) 
. comm - communicator argument to packing call (handle) 

Output Parameter:
. size - upper bound on size of packed message, in bytes (integer) 
@*/
int MPI_Pack_size ( incount, datatype, comm, size )
int           incount;
MPI_Datatype  datatype;
MPI_Comm      comm;
int          *size;
{
  int pad = 0;
  int mpi_errno;

  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,datatype) || 
      MPIR_TEST_ARG(size))
      return MPIR_ERROR(comm, mpi_errno, "Error in MPI_PACK_SIZE" );

  /* Figure out size needed to pack type and add the biggest size
	 of other types to give an upper bound */
  MPIR_Pack_size( incount, datatype, comm, size );
  (*size) += MPIR_I_DCOMPLEX.size;

return MPI_SUCCESS;
}


