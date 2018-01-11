/*
 *  $Id: address.c,v 1.9 1994/09/29 21:51:01 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: address.c,v 1.9 1994/09/29 21:51:01 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Address - Gets the address of a location in memory  

Input Parameters:
. location - location in caller memory (choice) 

Output Parameter:
. address - address of location (integer) 

    Note:
    This routine is provided for both the Fortran and C programmers.
    On many systems, the address returned by this routine will be the same
    as produced by the C '&' operator, but this is not required in C and
    may not be true of systems with word- rather than byte-oriented 
    instructions or systems with segmented address spaces.  
@*/
int MPI_Address( location, address)
void     *location;
MPI_Aint *address;
{
    *address = (MPI_Aint) ((char *)location - (char *)MPI_BOTTOM);
    return MPI_SUCCESS;
}
