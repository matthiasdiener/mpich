/*
 *  $Id: statusf2c.c,v 1.3 1998/01/16 16:25:36 swider Exp $
 *
 *  (C) 1997 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  MPI_Status_f2c - Convert a Fortran status to a C status

Input Parameters:
. f_status - Status value in Fortran (Integer)

Output Parameter:
. c_status - Status value in C (Status)
  
.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
@*/
int MPI_Status_f2c( f_status, c_status )
MPI_Fint     *f_status;
MPI_Status   *c_status;
{
    int i;
    int *c_status_arr = (int *)c_status;
    void *l_f_status = (void *)f_status;

    if  (l_f_status == MPIR_F_STATUS_IGNORE ||
	 l_f_status == MPIR_F_STATUSES_IGNORE) {
	 return MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_STATUS_IGNORE,
			   "MPI_STATUS_F2C" );
    }

    /* Copy Fortran to C values */
    for (i=0; i<MPI_STATUS_SIZE; i++)
	c_status_arr[i] = (int)f_status[i];
	
    return MPI_SUCCESS;
}
