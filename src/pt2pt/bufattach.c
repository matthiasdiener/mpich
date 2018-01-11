/*
 *  $Id: bufattach.c,v 1.7 1995/05/09 18:58:15 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: bufattach.c,v 1.7 1995/05/09 18:58:15 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
  MPI_Buffer_attach - Attaches a user-defined buffer for sending

Input Parameters:
. buffer - initial buffer address (choice) 
. size - buffer size, in bytes (integer) 

Notes:
The size given should be the sum of the sizes of all outstanding Bsends that
you intend to have, plus a few hundred bytes for each Bsend that you do.
For the purposes of calculating size, you should use MPI_Pack_size. 
In other words, in the code
.vb
     MPI_Buffer_attach( buffer, size );
     MPI_Bsend( ..., count=20, datatype=type1,  ... );
     ...
     MPI_Bsend( ..., count=40, datatype=type2, ... );
.ve
the value of 'size' in the MPI_Buffer_attach call should be greater than
the value computed by
.vb
     MPI_Pack_size( 20, type1, comm, &s1 );
     MPI_Pack_size( 40, type2, comm, &s2 );
     size = s1 + s2 + 2 * MPI_BSEND_OVERHEAD;
.ve    
The MPI_BSEND_OVERHEAD gives the maximum amount of space that may be used in 
the buffer for use by the BSEND routines in using the buffer.  This value 
is in 'mpi.h' (for C) and 'mpif.h' (for Fortran).

@*/
int MPI_Buffer_attach( buffer, size )
void *buffer;
int  size;
{
int mpi_errno;

if (mpi_errno = MPIR_SetBuffer( buffer, size ))
    return MPIR_ERROR( (MPI_Comm)0, mpi_errno, "Error in MPI_BUFFER_ATTACH" );

return MPI_SUCCESS;
}
