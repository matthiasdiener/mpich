/* testcancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

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
void mpi_test_cancelled_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_test_cancelled_( status, flag, __ierr )
MPI_Fint *status;
MPI_Fint *flag;
MPI_Fint *__ierr;
{
    int lflag;
    MPI_Status c_status;

    MPI_Status_f2c(status, &c_status); 
    *__ierr = MPI_Test_cancelled(&c_status, &lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
