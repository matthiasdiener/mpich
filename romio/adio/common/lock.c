/* 
 *   $Id: lock.c,v 1.2 1998/06/02 18:57:20 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

int ADIOI_Set_lock(int fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
    int err, error_code;
    struct flock lock;

    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    do {
	err = fcntl(fd, cmd, &lock);
    } while (err && (errno == EINTR));

    if (err && (errno != EBADF)) {
	printf("File locking failed in ADIOI_Set_lock\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
    return error_code;
}


#if (defined(__HFS) || defined(__XFS))
int ADIOI_Set_lock64(int fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
    int err, error_code;
    struct flock64 lock;

    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    do {
	err = fcntl(fd, cmd, &lock);
    } while (err && (errno == EINTR));

    if (err && (errno != EBADF)) {
	printf("File locking failed in ADIOI_Set_lock64\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
    return error_code;
}
#endif