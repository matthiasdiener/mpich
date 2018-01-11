/* 
 *   $Id: info_dupf.c,v 1.3 1998/04/29 16:59:17 swider Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_dup_ PMPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_dup_ pmpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_dup_ pmpi_info_dup
#else
#define mpi_info_dup_ pmpi_info_dup_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_dup_ MPI_INFO_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_dup_ mpi_info_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_dup_ mpi_info_dup
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_dup_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_info_dup_(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *__ierr )
{
    MPI_Info info_c, newinfo_c;

    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_dup(info_c, &newinfo_c);
    *newinfo = MPI_Info_c2f(newinfo_c);
}
