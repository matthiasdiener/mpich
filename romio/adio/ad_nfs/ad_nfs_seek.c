/* 
 *   $Id: ad_nfs_seek.c,v 1.2 1998/06/02 18:39:47 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"
#ifdef __PROFILE
#include "mpe.h"
#endif

ADIO_Offset ADIOI_NFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
