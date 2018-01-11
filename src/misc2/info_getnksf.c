/* 
 *   $Id: info_getnksf.c,v 1.3 1998/04/29 16:59:22 swider Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_info_get_nkeys_ PMPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys
#else
#define mpi_info_get_nkeys_ pmpi_info_get_nkeys_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_info_get_nkeys_ MPI_INFO_GET_NKEYS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_info_get_nkeys_ mpi_info_get_nkeys__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_info_get_nkeys_ mpi_info_get_nkeys
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_info_get_nkeys_ ANSI_ARGS((MPI_Fint *, MPI_Fint *, MPI_Fint *));

/* Definitions of Fortran Wrapper routines */
void mpi_info_get_nkeys_(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *__ierr )
{
    MPI_Info info_c;
    int l_nkeys;
    
    info_c = MPI_Info_f2c(*info);
    *__ierr = MPI_Info_get_nkeys(info_c, &l_nkeys);
    *nkeys = l_nkeys;
}
