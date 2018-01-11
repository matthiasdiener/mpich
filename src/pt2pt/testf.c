/* test.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_test_ PMPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_ pmpi_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_ pmpi_test
#else
#define mpi_test_ pmpi_test_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_test_ MPI_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_test_ mpi_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_test_ mpi_test
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_test_ ANSI_ARGS(( MPI_Request *, int *, MPI_Status *, int * ));

void mpi_test_ ( request, flag, status, __ierr )
MPI_Request  *request;
int          *flag;
MPI_Status   *status;
int *__ierr;
{
    MPI_Request lrequest = (MPI_Request) MPIR_ToPointer(*(int*)request);
    *__ierr = MPI_Test( &lrequest,flag,status);
    if (lrequest == MPI_REQUEST_NULL) {
	MPIR_RmPointer(*(int*)request);
	*(int*)request = 0;
    }
    *flag = MPIR_TO_FLOG(*flag);
}
