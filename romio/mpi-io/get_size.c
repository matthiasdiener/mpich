/* 
 *   $Id: get_size.c,v 1.2 1998/06/02 19:01:47 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

/*@
    MPI_File_get_size - Returns the file size

Input Parameters:
. fh - file handle (handle)

Output Parameters:
. size - size of the file in bytes (nonnegative integer)

.N fortran
@*/
int MPI_File_get_size(MPI_File fh, MPI_Offset *size)
{
    ADIO_Fcntl_t *fcntl_struct;
    int error_code;
#ifdef MPI_hpux
    int fl_xmpi;

    HPMP_IO_START(fl_xmpi, BLKMPIFILEGETSIZE, TRDTBLOCK, fh,
		  MPI_DATATYPE_NULL, -1);
#endif /* MPI_hpux */

    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	printf("MPI_File_get_size: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fcntl_struct = (ADIO_Fcntl_t *) ADIOI_Malloc(sizeof(ADIO_Fcntl_t));
    ADIO_Fcntl(fh, ADIO_FCNTL_GET_FSIZE, fcntl_struct, &error_code);
    *size = fcntl_struct->fsize;
    ADIOI_Free(fcntl_struct);

#ifdef MPI_hpux
    HPMP_IO_END(fl_xmpi, fh, MPI_DATATYPE_NULL, -1);
#endif /* MPI_hpux */
    return error_code;
}