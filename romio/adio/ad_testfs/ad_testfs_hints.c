/* 
 *   $Id: ad_testfs_hints.c,v 1.3 2001/08/01 15:29:43 rross Exp $    
 *
 *   Copyright (C) 2001 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_testfs.h"
#include "adioi.h"

void ADIOI_TESTFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    int myrank, nprocs;

    *error_code = MPI_SUCCESS;

    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &myrank);
    FPRINTF(stdout, "[%d/%d] ADIOI_TESTFS_SetInfo called on %s\n", 
	    myrank, nprocs, fd->filename);
    FPRINTF(stdout, "[%d/%d]    calling ADIOI_GEN_SetInfo\n", 
	    myrank, nprocs);

    ADIOI_GEN_SetInfo(fd, users_info, error_code);
}
