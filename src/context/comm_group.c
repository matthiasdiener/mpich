/*
 *  $Id: comm_group.c,v 1.1.1.1 1997/09/17 20:41:39 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@

MPI_Comm_group - Accesses the group associated with given communicator

Input Parameter:
. comm - Communicator

Output Parameter:
. group - Group in communicator

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
@*/
int MPI_Comm_group ( comm, group )
MPI_Comm comm;
MPI_Group *group;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_GROUP *new_group;
    static char myname[] = "MPI_COMM_GROUP";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    if (MPIR_TEST_COMM_NOTOK(comm,comm_ptr) ) {
	(*group) = MPI_GROUP_NULL;
	return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_COMM, myname);
    }
    else {
	MPIR_Group_dup( comm_ptr->local_group, &new_group );
	*group = new_group->self;
	TR_POP;
	return (MPI_SUCCESS);
    }
}
