/* 
 *   $Id: ad_delete.c,v 1.2 1998/06/02 18:55:58 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

void ADIO_Delete(char *filename, int *error_code)
{
    int err;

    err = unlink(filename);
    *error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
}
