/* 
 *   $Id: ad_sfs_flush.c,v 1.2 1998/06/02 18:48:34 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_sfs.h"

void ADIOI_SFS_Flush(ADIO_File fd, int *error_code)
{
     /* there is no no fsync on SX-4 */
     *error_code = MPI_ERR_UNKNOWN; 
}
