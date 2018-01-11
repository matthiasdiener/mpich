/* 
 *   $Id: ad_pfs_read.c,v 1.2 1998/06/02 18:43:05 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pfs.h"
#ifdef __PROFILE
#include "mpe.h"
#endif

void ADIOI_PFS_ReadContig(ADIO_File fd, void *buf, int len, int file_ptr_type,
		     ADIO_Offset offset, ADIO_Status *status, int *error_code)
{
    int err=-1;

    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
        if (fd->fp_sys_posn != offset) {
#ifdef __PROFILE
	    MPE_Log_event(11, 0, "start seek");
#endif
	    lseek(fd->fd_sys, offset, SEEK_SET);
#ifdef __PROFILE
	    MPE_Log_event(12, 0, "end seek");
#endif
	}
#ifdef __PROFILE
	MPE_Log_event(3, 0, "start read");
#endif
	err = _cread(fd->fd_sys, buf, len);
#ifdef __PROFILE
	MPE_Log_event(4, 0, "end read");
#endif
        fd->fp_sys_posn = offset + len;
	/* individual file pointer not updated */        
    }
    else {  /* read from curr. location of ind. file pointer */
        if (fd->fp_sys_posn != fd->fp_ind) {
#ifdef __PROFILE
	    MPE_Log_event(11, 0, "start seek");
#endif
	    lseek(fd->fd_sys, fd->fp_ind, SEEK_SET);
#ifdef __PROFILE
	    MPE_Log_event(12, 0, "end seek");
#endif
	}
#ifdef __PROFILE
	MPE_Log_event(3, 0, "start read");
#endif
	err = _cread(fd->fd_sys, buf, len);
#ifdef __PROFILE
	MPE_Log_event(4, 0, "end read");
#endif
	fd->fp_ind += len; 
        fd->fp_sys_posn = fd->fp_ind;
    }         

    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
}


void ADIOI_PFS_ReadStrided(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
    ADIOI_GEN_ReadStrided(fd, buf, count, datatype, file_ptr_type,
                        offset, status, error_code);
}