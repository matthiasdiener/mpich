/* 
 *   $Id: info_getvlnf.c,v 1.5 1998/04/29 16:59:25 swider Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_get_valuelen_ PMPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen
#else
#define mpi_info_get_valuelen_ pmpi_info_get_valuelen_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_get_valuelen_ MPI_INFO_GET_VALUELEN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_valuelen_ mpi_info_get_valuelen__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_valuelen_ mpi_info_get_valuelen
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_get_valuelen_ ANSI_ARGS((MPI_Fint *, char *, MPI_Fint *,
				       MPI_Fint *, MPI_Fint *, MPI_Fint));
/* Definitions of Fortran Wrapper routines */
void mpi_info_get_valuelen_(MPI_Fint *info, char *key, MPI_Fint *valuelen,
			    MPI_Fint *flag, MPI_Fint *__ierr, MPI_Fint keylen )
{
    MPI_Info info_c;
    char *newkey;
    int new_keylen, lead_blanks, i;
    int lvaluelen, lflag;

    if (key <= (char *) 0) {
        printf("MPI_Info_get_valuelen: key is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* strip leading and trailing blanks in key */
    lead_blanks = 0;
    for (i=0; i<(int)keylen; i++) 
        if (key[i] == ' ') lead_blanks++;
        else break;

    for (i=(int)keylen-1; i>=0; i--) if (key[i] != ' ') break;
    if (i < 0) {
        printf("MPI_Info_get_valuelen: key is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    new_keylen = i + 1 - lead_blanks;
    key += lead_blanks;

    newkey = (char *) MALLOC((new_keylen+1)*sizeof(char));
    strncpy(newkey, key, new_keylen);
    newkey[new_keylen] = '\0';

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_valuelen(info_c, newkey, &lvaluelen, &lflag);
    *valuelen = lvaluelen;
    *flag = MPIR_TO_FLOG(lflag);
    FREE(newkey);
}

