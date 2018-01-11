/*
 *  $Id: group_free.c,v 1.1.1.1 1997/09/17 20:41:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

/*@

MPI_Group_free - Frees a group

Input Parameter
. group - group (handle) 

Notes:
On output, group is set to 'MPI_GROUP_NULL'.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
.N MPI_ERR_PERM_GROUP
@*/
int MPI_Group_free ( group )
MPI_Group *group;
{
    struct MPIR_GROUP *group_ptr;
    int mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPI_GROUP_FREE";
    
    TR_PUSH(myname);

    /* Check for bad arguments */
    if ( MPIR_TEST_ARG(group) )
	return MPIR_ERROR( MPIR_COMM_WORLD, mpi_errno, myname );

    /* Free null groups generates error */
    if ( (*group) == MPI_GROUP_NULL ) {
	TR_POP;
	return MPIR_ERROR(MPIR_COMM_WORLD, MPI_ERR_GROUP, myname );
    }
	 
    group_ptr = MPIR_GET_GROUP_PTR(*group);
    MPIR_TEST_MPI_GROUP(*group,group_ptr,MPIR_COMM_WORLD,myname);

    /* We can't free permanent objects unless finalize has been called */
    if  ( ( group_ptr->permanent == 1 ) && group_ptr->ref_count <= 1 && 
          (MPIR_Has_been_initialized == 1) )
	return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_PERM_GROUP,myname );

    /* Free group */
    if ( group_ptr->ref_count <= 1 ) {
	MPIR_FreeGroup( group_ptr );
    }
    else {
	MPIR_REF_DECR(group_ptr);
    }
    /* This could be dangerous if the object is MPI_GROUP_EMPTY and not just
     a copy of it.... It would also be illegal. */
    (*group) = MPI_GROUP_NULL;

    TR_POP;
    return (mpi_errno);
}
