/* 
 *   $Id: ad_flush.c,v 1.2 1998/06/02 18:56:06 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

void ADIOI_GEN_Flush(ADIO_File fd, int *error_code)
{
    int err;

    err = fsync(fd->fd_sys);

    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
