/* 
 *   $Id: ad_pvfs_close.c,v 1.1.1.1 1999/08/06 17:47:47 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"

void ADIOI_PVFS_Close(ADIO_File fd, int *error_code)
{
    int err;

    err = pvfs_close(fd->fd_sys);
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
