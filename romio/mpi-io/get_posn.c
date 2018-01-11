/* 
 *   $Id: get_posn.c,v 1.2 1998/06/02 19:01:42 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

/*@
    MPI_File_get_position - Returns the current position of the 
                individual file pointer in etype units relative to
                the current view

Input Parameters:
. fh - file handle (handle)

Output Parameters:
. offset - offset of individual file pointer (nonnegative integer)

.N fortran
@*/
int MPI_File_get_position(MPI_File fh, MPI_Offset *offset)
{
    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	printf("MPI_File_get_position: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    ADIOI_Get_position(fh, offset);

    return MPI_SUCCESS;
}
