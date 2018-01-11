/* 
 *   $Id: read_allbf.c,v 1.7 2000/08/24 16:18:26 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"


#if defined(MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_read_all_begin_ PMPI_FILE_READ_ALL_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_read_all_begin_ pmpi_file_read_all_begin__
#elif !defined(FORTRANUNDERSCORE)
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_begin pmpi_file_read_all_begin_
#endif
#define mpi_file_read_all_begin_ pmpi_file_read_all_begin
#else
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_begin_ pmpi_file_read_all_begin
#endif
#define mpi_file_read_all_begin_ pmpi_file_read_all_begin_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_READ_ALL_BEGIN = PMPI_FILE_READ_ALL_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_read_all_begin__ = pmpi_file_read_all_begin__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_read_all_begin = pmpi_file_read_all_begin
#else
#pragma weak mpi_file_read_all_begin_ = pmpi_file_read_all_begin_
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_READ_ALL_BEGIN MPI_FILE_READ_ALL_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_begin__ mpi_file_read_all_begin__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_begin mpi_file_read_all_begin
#else
#pragma _HP_SECONDARY_DEF pmpi_file_read_all_begin_ mpi_file_read_all_begin_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_READ_ALL_BEGIN as PMPI_FILE_READ_ALL_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_read_all_begin__ as pmpi_file_read_all_begin__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_read_all_begin as pmpi_file_read_all_begin
#else
#pragma _CRI duplicate mpi_file_read_all_begin_ as pmpi_file_read_all_begin_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_read_all_begin_ MPI_FILE_READ_ALL_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_read_all_begin_ mpi_file_read_all_begin__
#elif !defined(FORTRANUNDERSCORE)
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_read_all_begin mpi_file_read_all_begin_
#endif
#define mpi_file_read_all_begin_ mpi_file_read_all_begin
#else
#if defined(HPUX) || defined(SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_read_all_begin_ mpi_file_read_all_begin
#endif
#endif
#endif

/* Prototype to keep compiler happy */
void mpi_file_read_all_begin_(MPI_Fint *fh,void *buf,int *count,
			      MPI_Datatype *datatype, int *ierr );

#if defined(MPIHP) || defined(MPILAM)
void mpi_file_read_all_begin_(MPI_Fint *fh,void *buf,int *count,
                      MPI_Fint *datatype,int *ierr )
{
    MPI_File fh_c;
    MPI_Datatype datatype_c;
    
    fh_c = MPI_File_f2c(*fh);
    datatype_c = MPI_Type_f2c(*datatype);

    *ierr = MPI_File_read_all_begin(fh_c,buf,*count,datatype_c);
}
#else
void mpi_file_read_all_begin_(MPI_Fint *fh,void *buf,int *count,
                      MPI_Datatype *datatype, int *ierr ){
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *ierr = MPI_File_read_all_begin(fh_c,buf,*count,*datatype);
}
#endif