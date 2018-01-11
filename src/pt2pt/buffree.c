/*
 *  $Id: buffree.c,v 1.7 1994/10/27 20:42:03 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: buffree.c,v 1.7 1994/10/27 20:42:03 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
  MPI_Buffer_detach - Removes an existing buffer (for use in MPI_Bsend etc)

Output Parameters:
. buffer - initial buffer address (choice) 
. size - buffer size, in bytes (integer) 

Notes:
    The reason that MPI_Buffer_detach returns the address and size of the
buffer being detached is to allow nested libraries to replace and restore
the buffer.  For example, consider

$    int size, mysize, idummy;
$    void *ptr, *myptr, *dummy;     
$    MPI_Buffer_detach( &ptr, &size );
$    MPI_Buffer_attach( myptr, mysize );
$    ...
$    ... library code ...
$    ...
$    MPI_Buffer_detach( &dummy, &idummy );
$    MPI_Buffer_attach( ptr, size );
$
This is much like the action of the Unix signal routine and has the same
strengths (its simple) and weaknesses (it only works for nested usages).

Notes for Fortran:
    The Fortran binding for this routine is different.  Because Fortran 
    does not have pointers, it is impossible to provide a way to use the
    output of this routine to exchange buffers.  In this case, only the
    size field is set.

@*/
int MPI_Buffer_detach( buffer, size )
void **buffer;
int  *size;
{
MPIR_FreeBuffer( buffer, size );

return MPI_SUCCESS;
}

