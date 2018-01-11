/* 
 *   $Id: ad_hfs_close.c,v 1.2 1998/06/02 18:35:28 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_hfs.h"

void ADIOI_HFS_Close(ADIO_File fd, int *error_code)
{
    int err;

    err = close(fd->fd_sys);
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
