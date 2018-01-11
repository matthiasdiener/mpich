/*
 *  $Id: getpnamef.c,v 1.3 1998/01/29 14:27:15 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
  
/*
 * Update log
 * Nov 29 1996 jcownie@dolphinics.com: Use MPIR_cstr2fstr to get the blank padding right.
 */

#include "mpiimpl.h"
#ifdef _CRAY
#include <fortran.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_get_processor_name_ PMPI_GET_PROCESSOR_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_processor_name_ pmpi_get_processor_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_processor_name_ pmpi_get_processor_name
#else
#define mpi_get_processor_name_ pmpi_get_processor_name_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_processor_name_ MPI_GET_PROCESSOR_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_processor_name_ mpi_get_processor_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_processor_name_ mpi_get_processor_name
#endif
#endif

#define LOCAL_MIN(a,b) ((a) < (b) ? (a) : (b))

/*
  MPI_GET_PROCESSOR_NAME - Gets the name of the processor for Fortran

*/
#ifdef _CRAY
void mpi_get_processor_name_( name_fcd, len, ierr )
int *len, *ierr;
_fcd name_fcd;
{
    char *name = _fcdtocp(name_fcd);
    long reslen= _fcdlen(name_fcd);
    char cres[MPI_MAX_PROCESSOR_NAME];

    MPID_Node_name( cres, MPI_MAX_PROCESSOR_NAME );

    /* This handles blank padding required by Fortran */
    MPIR_cstr2fstr(name, reslen, cres );
    *len  = LOCAL_MIN (strlen( cres ), reslen);
    *ierr = MPI_SUCCESS;
}

#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_get_processor_name_ ANSI_ARGS(( char *, MPI_Fint *, 
                                         MPI_Fint *, MPI_Fint ));

void mpi_get_processor_name_( name, len, ierr, d )
char     *name;
MPI_Fint *len;
MPI_Fint *ierr;
MPI_Fint d;


{
  char cres[MPI_MAX_PROCESSOR_NAME];
  int l_len;

    MPID_Node_name( cres, MPI_MAX_PROCESSOR_NAME );

    /* This handles blank padding required by Fortran */
    MPIR_cstr2fstr( name, (int)d, cres );
    l_len  = LOCAL_MIN( strlen( cres ), (int)d );
    *len = l_len;
    *ierr = MPI_SUCCESS;
}
#endif

