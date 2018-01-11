/* 
 *   $Id: ad_hfs_open.c,v 1.2 1998/06/02 18:36:37 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_hfs.h"

void ADIOI_HFS_Open(ADIO_File fd, int *error_code)
{
    int perm, old_mask, amode;

    if (fd->perm == ADIO_PERM_NULL) {
	old_mask = umask(022);
	umask(old_mask);
	perm = old_mask ^ 0666;
    }
    else perm = fd->perm;

    amode = 0;
    if (fd->access_mode & ADIO_CREATE)
	amode = amode | O_CREAT;
    if (fd->access_mode & ADIO_RDONLY)
	amode = amode | O_RDONLY;
    if (fd->access_mode & ADIO_WRONLY)
	amode = amode | O_WRONLY;
    if (fd->access_mode & ADIO_RDWR)
	amode = amode | O_RDWR;
    if (fd->access_mode & ADIO_EXCL)
	amode = amode | O_EXCL;

    fd->fd_sys = open64(fd->filename, amode, perm);

    if ((fd->fd_sys != -1) && (fd->access_mode & ADIO_APPEND)) {
	fd->fp_ind = lseek64(fd->fd_sys, 0, SEEK_END);
#ifdef __HPUX
	fd->fp_sys_posn = fd->fp_ind;
#endif
#ifdef __SPPUX
	fd->fp_sys_posn = -1;  /* set it to null bec. we use pread, pwrite*/
#endif
	MPI_Barrier(fd->comm);
	/* the barrier ensures that no process races ahead and modifies
           the file size before all processes have opened the file. */
    }

    *error_code = (fd->fd_sys == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
}