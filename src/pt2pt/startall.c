/*
 *  $Id: startall.c,v 1.3 1998/04/28 21:47:15 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Startall - Starts a collection of requests 

Input Parameters:
+ count - list length (integer) 
- array_of_requests - array of requests (array of handle) 

.N fortran
@*/
int MPI_Startall( count, array_of_requests )
int count;
MPI_Request array_of_requests[];
{
    int i;
    int mpi_errno;
    static char myname[] = "MPI_STARTALL";
    MPIR_ERROR_DECL;

    TR_PUSH(myname);

    MPIR_ERROR_PUSH(MPIR_COMM_WORLD);
    for (i=0; i<count; i++) {
	MPIR_CALL_POP(MPI_Start( array_of_requests + i ),
		      MPIR_COMM_WORLD,myname);
    }

    MPIR_ERROR_POP(MPIR_COMM_WORLD);
    TR_POP;
    return MPI_SUCCESS;
}
