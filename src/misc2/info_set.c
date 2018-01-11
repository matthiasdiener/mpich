/* 
 *   $Id: info_set.c,v 1.4 1998/04/28 21:25:07 swider Exp $    
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
    MPI_Info_set - Adds a (key,value) pair to info

Input Parameters:
+ info - info object (handle)
. key - key (string)
- value - value (string)

.N fortran
@*/
int MPI_Info_set(MPI_Info info, char *key, char *value)
{
    MPI_Info prev, curr;

    if ((info <= (MPI_Info) 0) || (info->cookie != MPIR_INFO_COOKIE)) {
        printf("MPI_Info_set: Invalid info object\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (key <= (char *) 0) {
	printf("MPI_Info_set: key is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (value <= (char *) 0) {
	printf("MPI_Info_set: value is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (strlen(key) > MPI_MAX_INFO_KEY) {
	printf("MPI_Info_set: key is longer than MPI_MAX_INFO_KEY\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (strlen(value) > MPI_MAX_INFO_VAL) {
	printf("MPI_Info_set: value is longer than MPI_MAX_INFO_VAL\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (!strlen(key)) {
	printf("MPI_Info_set: key is a null string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (!strlen(value)) {
	printf("MPI_Info_set: value is a null string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    prev = info;
    curr = info->next;

    while (curr) {
	if (!strcmp(curr->key, key)) {
#ifdef free
/* By default, we define free as an illegal expression when doing memory
   checking; we need to undefine it to handle the fact that strdup does
   a naked malloc.
 */
#undef free
#endif
	    free(curr->value);  /* not ADIOI_Free, because it was strdup'ed */
	    curr->value = strdup(value);
	    break;
	}
	prev = curr;
	curr = curr->next;
    }

    if (!curr) {
	prev->next = (MPI_Info) MALLOC(sizeof(struct MPIR_Info));
	curr = prev->next;
	curr->cookie = 0;  /* cookie not set on purpose */
	curr->key = strdup(key);
	curr->value = strdup(value);
	curr->next = 0;
    }

    return MPI_SUCCESS;
}