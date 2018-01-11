/* 
 *   $Id: info_create.c,v 1.3 1998/04/13 21:17:37 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*@
    MPI_Info_create - Creates a new info object

Output Parameters:
. info - info object (handle)

.N fortran
@*/
int MPI_Info_create(MPI_Info *info)
{
    *info	    = (MPI_Info) MALLOC(sizeof(struct MPIR_Info));
    (*info)->cookie = MPIR_INFO_COOKIE;
    (*info)->key    = 0;
    (*info)->value  = 0;
    (*info)->next   = 0;
    /* this is the first structure in this linked list. it is 
       always kept empty. new (key,value) pairs are added after it. */

    return MPI_SUCCESS;
}
