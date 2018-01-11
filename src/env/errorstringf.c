/* error_string.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
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

 void mpi_error_string_( errorcode, string, resultlen, __ierr )
int*errorcode, *resultlen;
char *string;
int *__ierr;
{
*__ierr = MPI_Error_string(*errorcode,string,resultlen);
}