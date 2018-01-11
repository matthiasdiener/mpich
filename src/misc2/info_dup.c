/* 
 *   $Id: info_dup.c,v 1.3 1998/04/13 21:17:39 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

#ifdef HAVE_STRING_H
/* For strdup */
#include <string.h> 
#endif

/*@
    MPI_Info_dup - Returns a duplicate of the info object

Input Parameters:
. info - info object (handle)

Output Parameters:
. newinfo - duplicate of info object (handle)

.N fortran
@*/
int MPI_Info_dup(MPI_Info info, MPI_Info *newinfo)
{
    MPI_Info curr_old, curr_new;

    if ((info <= (MPI_Info) 0) || (info->cookie != MPIR_INFO_COOKIE)) {
        printf("MPI_Info_dup: Invalid info object\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    *newinfo = (MPI_Info) MALLOC(sizeof(struct MPIR_Info));
    curr_new = *newinfo;
    curr_new->cookie = MPIR_INFO_COOKIE;
    curr_new->key = 0;
    curr_new->value = 0;
    curr_new->next = 0;

    curr_old = info->next;
    while (curr_old) {
	curr_new->next = (MPI_Info) MALLOC(sizeof(struct MPIR_Info));
	curr_new = curr_new->next;
	curr_new->cookie = 0;  /* cookie not set on purpose */
	curr_new->key = strdup(curr_old->key);
	curr_new->value = strdup(curr_old->value);
	curr_new->next = 0;
	
	curr_old = curr_old->next;
    }

    return MPI_SUCCESS;
}
