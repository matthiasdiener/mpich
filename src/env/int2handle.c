/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@C
  MPI_Int2handle - Convert an integer (Fortran) MPI handle to a C handle

Input Parameters:
. f_handle - Fortran integer handle
. handle_kind - Type of handle 

Return value:
. c_handle - C version of handle; should be cast to the correct type.

Notes:
The returned handle should be cast to the correct type by the user.

Notes for Fortran users:
There is no Fortran version of this routine.

.N MPI2
@*/
MPI_Handle_type MPI_Int2handle( f_handle, handle_kind )
MPI_Fint        f_handle;
MPI_Handle_enum handle_kind;
{
    switch (handle_kind) {
    case MPI_OP_HANDLE:
    case MPI_COMM_HANDLE:
    case MPI_DATATYPE_HANDLE:
    case MPI_ERRHANDLE_HANDLE:
    case MPI_GROUP_HANDLE:
	return (MPI_Handle_type) f_handle;
    default:
	/* Should only be requests */
	return (MPI_Handle_type)MPIR_ToPointer( f_handle );
    }
}
