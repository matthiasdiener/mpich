/* 
 *   $Id: ad_pfs_close.c,v 1.2 1998/06/02 18:41:14 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pfs.h"
#ifdef __PROFILE
#include "mpe.h"
#endif

void ADIOI_PFS_Close(ADIO_File fd, int *error_code)
{
    int err;

#ifdef __PROFILE
    MPE_Log_event(9, 0, "start close");
#endif
    err = close(fd->fd_sys);
#ifdef __PROFILE
    MPE_Log_event(10, 0, "end close");
#endif
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
