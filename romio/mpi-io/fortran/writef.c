/* 
 *   $Id: writef.c,v 1.6 1999/08/27 20:53:41 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpio.h"
#include "adio.h"


#if defined(__MPIO_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)
#ifdef FORTRANCAPS
#define mpi_file_write_ PMPI_FILE_WRITE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_write_ pmpi_file_write__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_write pmpi_file_write_
#endif
#define mpi_file_write_ pmpi_file_write
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF pmpi_file_write_ pmpi_file_write
#endif
#define mpi_file_write_ pmpi_file_write_
#endif

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_FILE_WRITE = PMPI_FILE_WRITE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_file_write__ = pmpi_file_write__
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_file_write = pmpi_file_write
#else
#pragma weak mpi_file_write_ = pmpi_file_write_
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_FILE_WRITE MPI_FILE_WRITE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_write__ mpi_file_write__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_file_write mpi_file_write
#else
#pragma _HP_SECONDARY_DEF pmpi_file_write_ mpi_file_write_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_FILE_WRITE as PMPI_FILE_WRITE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_file_write__ as pmpi_file_write__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_file_write as pmpi_file_write
#else
#pragma _CRI duplicate mpi_file_write_ as pmpi_file_write_
#endif

/* end of weak pragmas */
#endif
/* Include mapping from MPI->PMPI */
#include "mpioprof.h"
#endif

#else

#ifdef FORTRANCAPS
#define mpi_file_write_ MPI_FILE_WRITE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_file_write_ mpi_file_write__
#elif !defined(FORTRANUNDERSCORE)
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_write mpi_file_write_
#endif
#define mpi_file_write_ mpi_file_write
#else
#if defined(__HPUX) || defined(__SPPUX)
#pragma _HP_SECONDARY_DEF mpi_file_write_ mpi_file_write
#endif
#endif
#endif

#if defined(__MPIHP) || defined(__MPILAM)
void mpi_file_write_(MPI_Fint *fh,void *buf,int *count,
                   MPI_Fint *datatype,MPI_Status *status, int *__ierr )
{
    MPI_File fh_c;
    MPI_Datatype datatype_c;
    
    fh_c = MPI_File_f2c(*fh);
    datatype_c = MPI_Type_f2c(*datatype);

    *__ierr = MPI_File_write(fh_c, buf,*count,datatype_c,status);
}
#else
void mpi_file_write_(MPI_Fint *fh,void *buf,int *count,
                   MPI_Datatype *datatype,MPI_Status *status, int *__ierr )
{
    MPI_File fh_c;
    
    fh_c = MPI_File_f2c(*fh);
    *__ierr = MPI_File_write(fh_c, buf,*count,*datatype,status);
}
#endif
