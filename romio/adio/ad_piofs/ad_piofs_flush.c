/* 
 *   $Id: ad_piofs_flush.c,v 1.2 1998/06/02 18:44:58 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_piofs.h"

void ADIOI_PIOFS_Flush(ADIO_File fd, int *error_code)
{
    ADIOI_GEN_Flush(fd, error_code);
}
