/* 
 *   $Id: ad_hfs_resize.c,v 1.2 1998/06/02 18:37:08 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_hfs.h"

void ADIOI_HFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int err;
    
    err = ftruncate64(fd->fd_sys, size);
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
