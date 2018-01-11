/* 
 *   $Id: ad_pfs_done.c,v 1.2 1998/06/02 18:41:23 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pfs.h"

int ADIOI_PFS_ReadDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    int done=0;

    if (*request == ADIO_REQUEST_NULL) {
        *error_code = MPI_SUCCESS;
        return 1;
    }

    if ((*request)->next != ADIO_REQUEST_NULL) {
        done = ADIOI_PFS_ReadDone(&((*request)->next), status, error_code);
    /* currently passing status and error_code here, but something else
       needs to be done to get the status and error info correctly */
        if (!done) {
           *error_code = MPI_SUCCESS;
           return done;
        }
    }
    
    if ((*request)->queued)
	done = _iodone(*((long *) (*request)->handle));
    else done = 1; /* ADIOI_Complete_Async completed this request, 
                      but request object was not freed. */

    if (done == 1) {
        /* if request is still queued in the system, it is also there
           on ADIOI_Async_list. Delete it from there. */
        if ((*request)->queued) ADIOI_Del_req_from_list(request);

        (*request)->fd->async_count--;
        if ((*request)->handle) ADIOI_Free((*request)->handle);
        ADIOI_Free_request((ADIOI_Req_node *) (*request));
        *request = ADIO_REQUEST_NULL;
        /* status to be filled */
    }
    
    *error_code = (done == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
    return done;
}


int ADIOI_PFS_WriteDone(ADIO_Request *request, ADIO_Status *status, int *error_code)  
{
    return ADIOI_PFS_ReadDone(request, status, error_code);
} 
