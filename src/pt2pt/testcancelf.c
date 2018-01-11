/* testcancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_test_cancelled_ PMPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_cancelled_ pmpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_cancelled_ pmpi_test_cancelled
#else
#define mpi_test_cancelled_ pmpi_test_cancelled_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_test_cancelled_ MPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_cancelled_ mpi_test_cancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_cancelled_ mpi_test_cancelled
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_test_cancelled_ ANSI_ARGS(( MPI_Status *, int *, int * ));

void mpi_test_cancelled_( status, flag, __ierr )
MPI_Status*status;
int        *flag;
int *__ierr;
{
    *__ierr = MPI_Test_cancelled(status,flag);
    *flag = MPIR_TO_FLOG(*flag);
}
