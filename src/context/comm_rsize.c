/*
 *  $Id: comm_rsize.c,v 1.1.1.1 1997/09/17 20:41:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*@

MPI_Comm_remote_size - Determines the size of the remote group 
                       associated with an inter-communictor

Input Parameter:
. comm - communicator (handle) 

Output Parameter:
. size - number of processes in the group of 'comm'  (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_ARG
@*/
int MPI_Comm_remote_size ( comm, size )
MPI_Comm  comm;
int      *size;
{
    int mpi_errno;
    struct MPIR_COMMUNICATOR *comm_ptr;
    int flag;
    static char myname[] = "MPI_COMM_REMOTE_SIZE";

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    /* Check for intra-communicator */
    MPI_Comm_test_inter ( comm, &flag );
    if (!flag) return MPIR_ERROR(comm_ptr,MPI_ERR_COMM,
		       "Intra-communicator invalid in MPI_COMM_REMOTE_SIZE");

    if (MPIR_TEST_ARG(size)) {
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );
    }
    else 
	(*size) = comm_ptr->group->np;

    return (MPI_SUCCESS);
}
