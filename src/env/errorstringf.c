/* error_string.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef _CRAY
#include "fortran.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_error_string_ PMPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_string_ pmpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_string_ pmpi_error_string
#else
#define mpi_error_string_ pmpi_error_string_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_error_string_ MPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_string_ mpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_string_ mpi_error_string
#endif
#endif

#ifdef _CRAY
 void mpi_error_string_( errorcode, string_fcd, resultlen, __ierr )
int*errorcode, *resultlen;
_fcd string_fcd;
int *__ierr;
{
char *string;
string = _fcdtocp(string_fcd);
*__ierr = MPI_Error_string(*errorcode,string,resultlen);
}
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_error_string_ ANSI_ARGS(( int *, char *, int *, int *, int ));
void mpi_error_string_( errorcode, string, resultlen, __ierr, d )
int*errorcode, *resultlen;
char *string;
int *__ierr;
int d;
{
    *__ierr = MPI_Error_string(*errorcode,string,resultlen);
}
#endif
