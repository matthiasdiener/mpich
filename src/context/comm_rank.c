/*
 *  $Id: comm_rank.c,v 1.17 1997/01/07 01:47:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@

MPI_Comm_rank - Determines the rank of the calling process in the communicator

Input Parameters:
. comm - communicator (handle) 

Output Parameter:
. rank - rank of the calling process in group of  'comm'  (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
@*/
int MPI_Comm_rank ( comm, rank )
MPI_Comm  comm;
int      *rank;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_COMM_RANK";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname );

    (*rank) = comm_ptr->local_group->local_rank;

    TR_POP;
    return (MPI_SUCCESS);
}
