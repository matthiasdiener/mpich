/* errcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_errhandler_create_ PMPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_create_ pmpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_create_ pmpi_errhandler_create
#else
#define mpi_errhandler_create_ pmpi_errhandler_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_create_ MPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_create_ mpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_create_ mpi_errhandler_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
void mpi_errhandler_create_ ANSI_ARGS(( MPI_Handler_function **, 
					MPI_Errhandler *, int * ));
#else
void mpi_errhandler_create_ ANSI_ARGS(( MPI_Handler_function *, 
					MPI_Errhandler *, int * ));
#endif

void mpi_errhandler_create_( function, errhandler, __ierr )
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
MPI_Handler_function **function;
#else
MPI_Handler_function *function;
#endif
MPI_Errhandler       *errhandler;
int *__ierr;
{
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPI_Errhandler_create( *function, errhandler );
#else
    *__ierr = MPI_Errhandler_create( function, errhandler );
#endif
}
