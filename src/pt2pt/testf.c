/* test.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

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
void mpi_test_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint * ));

void mpi_test_ ( request, flag, status, __ierr )
MPI_Fint *request;
MPI_Fint *flag;
MPI_Fint *status;
MPI_Fint *__ierr;
{
    int        l_flag;
    MPI_Status c_status;
    MPI_Request lrequest = MPI_Request_f2c(*request);

    *__ierr = MPI_Test( &lrequest, &l_flag, &c_status);

#ifdef OLD_POINTER
    if (lrequest == MPI_REQUEST_NULL) {
	MPIR_RmPointer((int)lrequest);
	*request = 0;
    }
    else
#endif
        *request = MPI_Request_c2f(lrequest);

    *flag = MPIR_TO_FLOG(l_flag);
    if (l_flag) 
	MPI_Status_c2f(&c_status, status);
}
