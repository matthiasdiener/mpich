/* 
 *   $Id: info_getnthf.c,v 1.5 1998/07/01 19:56:14 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif


#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_get_nthkey_ PMPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey
#else
#define mpi_info_get_nthkey_ pmpi_info_get_nthkey_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_get_nthkey_ MPI_INFO_GET_NTHKEY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nthkey_ mpi_info_get_nthkey__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nthkey_ mpi_info_get_nthkey
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_get_nthkey_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, char *, 
				     MPI_Fint *, MPI_Fint));

/* Definitions of Fortran Wrapper routines */
void mpi_info_get_nthkey_(MPI_Fint *info, MPI_Fint *n, char *key, 
			  MPI_Fint *__ierr, MPI_Fint keylen)
{
    MPI_Info info_c;
    int i, tmpkeylen;
    char *tmpkey;

    if (key <= (char *) 0) {
        printf("MPI_Info_get_nthkey: key is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    tmpkey = (char *) MALLOC((MPI_MAX_INFO_KEY+1) * sizeof(char));
    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_nthkey(info_c, (int)*n, tmpkey);

    tmpkeylen = strlen(tmpkey);

    if (tmpkeylen <= (int)keylen) {
	strncpy(key, tmpkey, tmpkeylen);

	/* blank pad the remaining space */
	for (i=tmpkeylen; i<(int)keylen; i++) key[i] = ' ';
    }
    else {
	/* not enough space */
	strncpy(key, tmpkey, (int)keylen);
	/* this should be flagged as an error. */
	*__ierr = MPI_ERR_UNKNOWN;
    }

    FREE(tmpkey);
}


