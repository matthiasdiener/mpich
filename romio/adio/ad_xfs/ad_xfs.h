/* 
 *   $Id: ad_xfs.h,v 1.2 1998/06/02 18:54:17 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifndef __AD_XFS_INCLUDE
#define __AD_XFS_INCLUDE

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "adio.h"
#include <aio.h>

int ADIOI_XFS_aio(ADIO_File fd, void *buf, int len, ADIO_Offset offset,
		  int wr, void *handle);

#ifdef __HAS_PREAD64
#  define pread pread64
#  define pwrite pwrite64
#endif
/* above needed for IRIX 6.5 */

#endif