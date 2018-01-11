/*
 *  $Id: statusc2f.c,v 1.2 1997/12/08 22:26:56 gropp Exp $
 *
 *  (C) 1997 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Status_c2f - Convert a C status to a Fortran status

Input Parameters:
. c_status - Status value in C (Status)

Output Parameter:
. f_status - Status value in Fortran (Integer)
  
.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
@*/
int MPI_Status_c2f( c_status, f_status )
MPI_Status   *c_status;
MPI_Fint     *f_status;
{
    int i;
    int *c_status_arr = (int *)c_status;

    if (c_status == MPI_STATUS_IGNORE ||
	c_status == MPI_STATUSES_IGNORE) {
	return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_STATUS_IGNORE,
			   "MPI_STATUS_C2F" );
    }

    /* Copy C to Fortran */
    for (i=0; i<MPI_STATUS_SIZE; i++)
	f_status[i] = (MPI_Fint)c_status_arr[i];
	
    return MPI_SUCCESS;
}
