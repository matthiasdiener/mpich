/* 
 *   $Id: info_freef.c,v 1.4 1998/05/04 17:53:55 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_free_ PMPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_free_ pmpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_free_ pmpi_info_free
#else
#define mpi_info_free_ pmpi_info_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_free_ MPI_INFO_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_free_ mpi_info_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_free_ mpi_info_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_free_ ANSI_ARGS((MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_info_free_(MPI_Fint *info, MPI_Fint *__ierr )
{
    MPI_Info info_c;

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_free(&info_c);
    *info = MPI_Info_c2f(info_c);
}
