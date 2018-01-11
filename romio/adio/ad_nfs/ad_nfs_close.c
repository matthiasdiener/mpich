/* 
 *   $Id: ad_nfs_close.c,v 1.2 1998/06/02 18:38:22 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"

void ADIOI_NFS_Close(ADIO_File fd, int *error_code)
{
    int err;
    
    err = close(fd->fd_sys);
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
