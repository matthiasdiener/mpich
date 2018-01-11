/* 
 *   $Id: read_allf.c,v 1.2 1998/06/02 19:06:28 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"

#ifdef __MPIO_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_file_read_all_ PMPI_FILE_READ_ALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_read_all_ pmpi_file_read_all__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all pmpi_file_read_all_
#endif
#define mpi_file_read_all_ pmpi_file_read_all
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_ pmpi_file_read_all
#endif
#define mpi_file_read_all_ pmpi_file_read_all_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_file_read_all_ MPI_FILE_READ_ALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_read_all_ mpi_file_read_all__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_read_all mpi_file_read_all_
#endif
#define mpi_file_read_all_ mpi_file_read_all
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_read_all_ mpi_file_read_all
#endif
#endif
#endif

#ifdef __MPIHP
void mpi_file_read_all_(MPI_Fint *fh,void *buf,int *count,
                      MPI_Fint *datatype,MPI_Status *status, int *__ierr )
{
    MPI_File fh_c;
    MPI_Datatype datatype_c;
    
    fh_c = MPI_File_f2c(*fh);
    datatype_c = MPI_Type_f2c(*datatype);

    *__ierr = MPI_File_read_all(fh_c,buf,*count,datatype_c,status);
}
#else
void mpi_file_read_all_(MPI_Fint *fh,void *buf,int *count,
                      MPI_Datatype *datatype,MPI_Status *status, int *__ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *__ierr = MPI_File_read_all(fh_c,buf,*count,*datatype,status);
}
#endif