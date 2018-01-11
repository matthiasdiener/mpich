/* 
 *   $Id: info_createf.c,v 1.3 1998/04/29 16:59:15 swider Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_create_ PMPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_create_ pmpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_create_ pmpi_info_create
#else
#define mpi_info_create_ pmpi_info_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_create_ MPI_INFO_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_create_ mpi_info_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_create_ mpi_info_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_create_ ANSI_ARGS((MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_info_create_(MPI_Fint *info, MPI_Fint *__ierr )
{
    MPI_Info info_c;

    *__ierr = MPI_Info_create(&info_c);
    *info = MPI_Info_c2f(info_c);
}
