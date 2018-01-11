/* testcancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_testcancelled_ PMPI_TESTCANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testcancelled_ pmpi_testcancelled__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testcancelled_ pmpi_testcancelled
#else
#define mpi_testcancelled_ pmpi_testcancelled_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_testcancelled_ MPI_TEST_CANCELLED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testcancelled_ mpi_test_cancelled__
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
