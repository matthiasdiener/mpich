/* 
 *   $Id: info_get.c,v 1.3 1998/07/01 19:56:13 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif

/*@
    MPI_Info_get - Retrieves the value associated with a key

Input Parameters:
+ info - info object (handle)
. key - key (string)
- valuelen - length of value argument (integer)

Output Parameters:
+ value - value (string)
- flag - true if key defined, false if not (boolean)

.N fortran
@*/
int MPI_Info_get(MPI_Info info, char *key, int valuelen, char *value, int *flag)
{
    MPI_Info curr;

    if ((info <= (MPI_Info) 0) || (info->cookie != MPIR_INFO_COOKIE)) {
        printf("MPI_Info_get: Invalid info object\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (key <= (char *) 0) {
	printf("MPI_Info_get: key is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (strlen(key) > MPI_MAX_INFO_KEY) {
	printf("MPI_Info_get: key is longer than MPI_MAX_INFO_KEY\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (!strlen(key)) {
	printf("MPI_Info_get: key is a null string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (valuelen <= 0) {
	printf("MPI_Info_get: Invalid valuelen argument\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (value <= (char *) 0) {
	printf("MPI_Info_get: value is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    curr = info->next;
    *flag = 0;

    while (curr) {
	if (!strcmp(curr->key, key)) {
	    strncpy(value, curr->value, valuelen);
	    value[valuelen] = '\0';
	    *flag = 1;
	    break;
	}
	curr = curr->next;
    }

    return MPI_SUCCESS;
}