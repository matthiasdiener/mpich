/*
 *  $Id: comm_rgroup.c,v 1.1.1.1 1997/09/17 20:41:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_remote_group - Accesses the remote group associated with 
                        the given inter-communicator

Input Parameter:
. comm - Communicator (must be intercommunicator)

Output Parameter:
. group - remote group of communicator

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
@*/
int MPI_Comm_remote_group ( comm, group )
MPI_Comm comm;
MPI_Group *group;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_GROUP *group_ptr;
    int flag;
    static char myname[] = "MPI_COMM_REMOTE_GROUP";

    TR_PUSH(myname);
    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname );

    /* Check for intra-communicator */
    MPI_Comm_test_inter ( comm, &flag );
    if (!flag) return MPIR_ERROR(comm_ptr,MPI_ERR_COMM,
		       "Intra-communicator invalid in MPI_COMM_REMOTE_GROUP");

    MPIR_Group_dup( comm_ptr->group, &group_ptr );
    *group = group_ptr->self;
    TR_POP;
    return (MPI_SUCCESS);
}
