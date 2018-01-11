/* testcancel.c */
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

void mpi_test_cancelled_( status, flag, __ierr )
MPI_Status*status;
int        *flag;
int *__ierr;
{
*__ierr = MPI_Test_cancelled(status,flag);
*flag = MPIR_TO_FLOG(*flag);
}
