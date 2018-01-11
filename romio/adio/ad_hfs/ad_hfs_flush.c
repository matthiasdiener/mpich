/* 
 *   $Id: ad_hfs_flush.c,v 1.2 1998/06/02 18:35:44 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_hfs.h"

void ADIOI_HFS_Flush(ADIO_File fd, int *error_code)
{
    ADIOI_GEN_Flush(fd, error_code);
}
