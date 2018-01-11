/* 
 *   $Id: info_getf.c,v 1.6 1998/07/01 19:56:13 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif


#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_get_ PMPI_INFO_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_ pmpi_info_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_ pmpi_info_get
#else
#define mpi_info_get_ pmpi_info_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_get_ MPI_INFO_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_ mpi_info_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_ mpi_info_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_get_ ANSI_ARGS((MPI_Fint *, char *, MPI_Fint *, char *,
			      MPI_Fint *, MPI_Fint *, MPI_Fint, MPI_Fint));

/* Definitions of Fortran Wrapper routines */
void mpi_info_get_(MPI_Fint *info, char *key, MPI_Fint *valuelen, char *value, 
        MPI_Fint *flag, MPI_Fint *__ierr, MPI_Fint keylen, MPI_Fint valspace)
{
    MPI_Info info_c;
    char *newkey, *tmpvalue;
    int new_keylen, lead_blanks, i, tmpvaluelen;
    int lflag;

    if (key <= (char *) 0) {
        printf("MPI_Info_get: key is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* strip leading and trailing blanks in key */
    lead_blanks = 0;
    for (i=0; i<(int)keylen; i++) 
        if (key[i] == ' ') lead_blanks++;
        else break;

    for (i=(int)keylen-1; i>=0; i--) if (key[i] != ' ') break;
    if (i < 0) {
        printf("MPI_Info_get: key is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    new_keylen = i + 1 - lead_blanks;
    key += lead_blanks;

    newkey = (char *) MALLOC((new_keylen+1)*sizeof(char));
    strncpy(newkey, key, new_keylen);
    newkey[new_keylen] = '\0';

    if (value <= (char *) 0) {
        printf("MPI_Info_get: value is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (*valuelen <= 0) {
        printf("MPI_Info_get: Invalid valuelen argument\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if ((int)*valuelen > (int)valspace) {
        printf("MPI_Info_get: valuelen is greater than the amount of memory available in value\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    tmpvalue = (char *) MALLOC(((int)*valuelen + 1)*sizeof(char));

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get(info_c, newkey, (int)*valuelen, tmpvalue, &lflag);

    if (lflag) {
	tmpvaluelen = strlen(tmpvalue);
	strncpy(value, tmpvalue, tmpvaluelen);
	/* blank pad the remaining space */
	for (i=tmpvaluelen; i<(int)valspace; i++) value[i] = ' ';
    }
    *flag = MPIR_TO_FLOG(lflag);
    FREE(newkey);
    FREE(tmpvalue);
}
