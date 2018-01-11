/* error_string.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef _CRAY
#include "fortran.h"
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

#define LOCAL_MIN(a,b) ((a) < (b) ? (a) : (b))

#ifdef _CRAY
void mpi_error_string_( errorcode, string_fcd, resultlen, __ierr )
int*errorcode, *resultlen;
_fcd string_fcd;
int *__ierr;
{
  char cres[MPI_MAX_ERROR_STRING];
  *__ierr = MPI_Error_string(*errorcode,cres,resultlen);
 
  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(_fcdtocp(string_fcd), _fcdlen(string_fcd), cres);
  
  *resultlen = LOCAL_MIN(_fcdlen(string_fcd), *resultlen);
}
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_error_string_ ANSI_ARGS(( MPI_Fint *, char *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint ));
void mpi_error_string_( errorcode, string, resultlen, __ierr, d )
MPI_Fint *errorcode; 
char     *string;
MPI_Fint *resultlen;
MPI_Fint *__ierr;
MPI_Fint d;
{
  char cres[MPI_MAX_ERROR_STRING];
  int l_resultlen;

  *__ierr = MPI_Error_string((int)*errorcode, cres, &l_resultlen);

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(string, (int)d, cres);
  *resultlen = LOCAL_MIN((MPI_Fint)l_resultlen, (int)d);
}
#endif
