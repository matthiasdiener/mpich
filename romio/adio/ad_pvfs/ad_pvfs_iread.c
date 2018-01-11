/* 
 *   $Id: ad_pvfs_iread.c,v 1.1.1.1 1999/08/06 17:47:46 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"

void ADIOI_PVFS_IreadContig(ADIO_File fd, void *buf, int len, int file_ptr_type,
                ADIO_Offset offset, ADIO_Request *request, int *error_code)  
{
    ADIO_Status status;

/* PVFS does not support nonblocking I/O. Therefore, use blocking I/O */

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    (*request)->queued = 0;

    ADIOI_PVFS_ReadContig(fd, buf, len, file_ptr_type, offset, &status,
		    error_code);  

    fd->async_count++;

/* status info. must be linked to the request structure, so that it
   can be accessed later from a wait */
}



void ADIOI_PVFS_IreadStrided(ADIO_File fd, void *buf, int count, 
		       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Request *request, int
                       *error_code)
{
    ADIO_Status status;

/* PVFS does not support nonblocking I/O. Therefore, use blocking I/O */

    *request = ADIOI_Malloc_request();
    (*request)->optype = ADIOI_READ;
    (*request)->fd = fd;
    (*request)->next = ADIO_REQUEST_NULL;
    (*request)->queued = 0;

    ADIOI_PVFS_ReadStrided(fd, buf, count, datatype, file_ptr_type, 
                            offset, &status, error_code);  

    fd->async_count++;

/* status info. must be linked to the request structure, so that it
   can be accessed later from a wait */

}
