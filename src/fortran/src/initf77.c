#include "mpi_fortimpl.h"

/* Fortran logical values */
MPI_Fint MPIR_F_TRUE, MPIR_F_FALSE;

/* 
 Location of the Fortran marker for MPI_BOTTOM.  The Fortran wrappers
 must detect the use of this address and replace it with MPI_BOTTOM.
 This is done by the macro MPIR_F_PTR.
 */
void *MPIR_F_MPI_BOTTOM = 0;
/* Here are the special status ignore values in MPI-2 */
void *MPIR_F_STATUS_IGNORE = 0;
void *MPIR_F_STATUSES_IGNORE = 0;

void MPIR_Init_f77( void )
{
    mpir_init_flog_( &MPIR_F_TRUE, &MPIR_F_FALSE );
    /* fcm sets MPI_BOTTOM */
    mpir_init_fcm_( );
}
