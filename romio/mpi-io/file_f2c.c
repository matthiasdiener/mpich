/* 
 *   $Id: file_f2c.c,v 1.2 1998/06/02 19:00:59 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"
#include "adio_extern.h"

/*@
    MPI_File_f2c - Translates a Fortran file handle to a C file handle

Input Parameters:
. fh - Fortran file handle (integer)

Return Value:
  C file handle (handle)
@*/
MPI_File MPI_File_f2c(MPI_Fint fh)
{

#ifndef __INT_LT_POINTER
    return (MPI_File) ((void *) fh);  
    /* the extra cast is to get rid of a compiler warning on Exemplar.
       The warning is because MPI_File points to a structure containing
       longlongs, which may be 8-byte aligned. But MPI_Fint itself
       may not be 8-byte aligned.*/
#else
    if (!fh) return MPI_FILE_NULL;
    if ((fh < 0) || (fh > ADIOI_Ftable_ptr)) {
	printf("MPI_File_f2c: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    return ADIOI_Ftable[fh];
#endif
}
