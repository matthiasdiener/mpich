/* 
 *   $Id: set_viewf.c,v 1.2 1998/06/02 19:06:58 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"

#ifdef __MPIO_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_file_set_view_ PMPI_FILE_SET_VIEW
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_set_view_ pmpi_file_set_view__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_set_view pmpi_file_set_view_
#endif
#define mpi_file_set_view_ pmpi_file_set_view
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_set_view_ pmpi_file_set_view
#endif
#define mpi_file_set_view_ pmpi_file_set_view_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_file_set_view_ MPI_FILE_SET_VIEW
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_set_view_ mpi_file_set_view__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_set_view mpi_file_set_view_
#endif
#define mpi_file_set_view_ mpi_file_set_view
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_set_view_ mpi_file_set_view
#endif
#endif
#endif

#ifdef __MPIHP
void mpi_file_set_view_(MPI_Fint *fh,MPI_Offset *disp,MPI_Fint *etype,
   MPI_Fint *filetype,char *datarep,MPI_Fint *info, int *__ierr,
   int str_len )
{
    char *newstr;
    MPI_File fh_c;
    int i, real_len; 
    MPI_Datatype etype_c, filetype_c;
    MPI_Info info_c;
    
    etype_c = MPI_Type_f2c(*etype);
    filetype_c = MPI_Type_f2c(*filetype);
    info_c = MPI_Info_f2c(*info);

    /* strip trailing blanks in datarep */
    if (datarep <= (char *) 0) {
        printf("MPI_File_set_view: datarep is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (i=str_len-1; i>=0; i--) if (datarep[i] != ' ') break;
    if (i < 0) {
	printf("MPI_File_set_view: datarep is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    real_len = i + 1;

    newstr = (char *) ADIOI_Malloc((real_len+1)*sizeof(char));
    strncpy(newstr, datarep, real_len);
    newstr[real_len] = '\0';
    
    fh_c = MPI_File_f2c(*fh);
 
    *__ierr = MPI_File_set_view(fh_c,*disp,etype_c,filetype_c,newstr,info_c);

    ADIOI_Free(newstr);
}

#else

void mpi_file_set_view_(MPI_Fint *fh,MPI_Offset *disp,MPI_Datatype *etype,
   MPI_Datatype *filetype,char *datarep,MPI_Fint *info, int *__ierr,
   int str_len )
{
    char *newstr;
    MPI_File fh_c;
    int i, real_len; 
    MPI_Info info_c;
    
    info_c = MPI_Info_f2c(*info);

    /* strip trailing blanks in datarep */
    if (datarep <= (char *) 0) {
        printf("MPI_File_set_view: datarep is an invalid address\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (i=str_len-1; i>=0; i--) if (datarep[i] != ' ') break;
    if (i < 0) {
	printf("MPI_File_set_view: datarep is a blank string\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    real_len = i + 1;

    newstr = (char *) ADIOI_Malloc((real_len+1)*sizeof(char));
    strncpy(newstr, datarep, real_len);
    newstr[real_len] = '\0';
    
    fh_c = MPI_File_f2c(*fh);
 
    *__ierr = MPI_File_set_view(fh_c,*disp,*etype,*filetype,newstr,info_c);

    ADIOI_Free(newstr);
}
#endif