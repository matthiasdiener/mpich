/*
 *  $Id: address.c,v 1.2 1998/01/29 14:27:37 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

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

.N fortran
@*/
int MPI_Address( location, address)
void     *location;
MPI_Aint *address;
{
  /* SX_4 needs to set CHAR_PTR_IS_ADDRESS 
     The reason is that it computes the different in two pointers in
     an "int", and addresses typically have the high (bit 31) bit set;
     thus the difference, when cast as MPI_Aint (long), is sign-extended, 
     making the absolute address negative.  Without a copy of the C 
     standard, I can't tell if this is a compiler bug or a language bug.
    */
#ifdef CHAR_PTR_IS_ADDRESS
    *address = (MPI_Aint) ((char *)location);
#else
    /* Note that this is the "portable" way to generate an address.
       The difference of two pointers is the number of elements
       between them, so this gives the number of chars between location
       and ptr.  As long as sizeof(char) == 1, this will be the number
       of bytes from 0 to location */
    *address = (MPI_Aint) ((char *)location - (char *)MPI_BOTTOM);
#endif
    return MPI_SUCCESS;
}
