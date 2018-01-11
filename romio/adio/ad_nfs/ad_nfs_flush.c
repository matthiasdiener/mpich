/* 
 *   $Id: ad_nfs_flush.c,v 1.2 1998/06/02 18:38:55 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_nfs.h"

void ADIOI_NFS_Flush(ADIO_File fd, int *error_code)
{
    ADIOI_GEN_Flush(fd, error_code);
}
