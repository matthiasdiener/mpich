/* errclass.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_error_class_ PMPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_class_ pmpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_class_ pmpi_error_class
#else
#define mpi_error_class_ pmpi_error_class_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_error_class_ MPI_ERROR_CLASS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_class_ mpi_error_class__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_class_ mpi_error_class
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_error_class_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_error_class_( errorcode, errorclass, __ierr )
MPI_Fint *errorcode; 
MPI_Fint *errorclass;
MPI_Fint *__ierr;
{
    int l_errorclass;

    *__ierr = MPI_Error_class((int)*errorcode, &l_errorclass);
    *errorclass = l_errorclass;
}
