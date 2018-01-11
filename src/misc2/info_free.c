/* 
 *   $Id: info_free.c,v 1.3 1998/04/10 17:35:33 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*@
    MPI_Info_free - Frees an info object

Input Parameters:
. info - info object (handle)

.N fortran
@*/
int MPI_Info_free(MPI_Info *info)
{
    MPI_Info curr, next;

    if ((*info <= (MPI_Info) 0) || ((*info)->cookie != MPIR_INFO_COOKIE)) {
        printf("MPI_Info_free: Invalid info object\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    curr = (*info)->next;
    FREE(*info);
    *info = MPI_INFO_NULL;

    while (curr) {
	next = curr->next;
#ifdef free
/* By default, we define free as an illegal expression when doing memory
   checking; we need to undefine it to handle the fact that strdup does
   a naked malloc.
 */
#undef free
#endif
	free(curr->key);
	free(curr->value);
	FREE(curr);
	curr = next;
    }

    return MPI_SUCCESS;
}
