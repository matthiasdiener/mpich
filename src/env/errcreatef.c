/* errcreate.c */
/* Custom Fortran interface file */
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

void mpi_errhandler_create_( function, errhandler, __ierr )
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
MPI_Handler_function **function;
#else
MPI_Handler_function *function;
#endif
MPI_Errhandler       *errhandler;
int *__ierr;
{
MPI_Errhandler new;
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
*__ierr = MPI_Errhandler_create( *function, &new );
#else
*__ierr = MPI_Errhandler_create( function, &new );
#endif
*(int *)errhandler = MPIR_FromPointer(new);
}
