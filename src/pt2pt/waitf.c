/* wait.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_wait_ PMPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wait_ pmpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wait_ pmpi_wait
#else
#define mpi_wait_ pmpi_wait_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_wait_ MPI_WAIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wait_ mpi_wait__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wait_ mpi_wait
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_wait_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_wait_ ( request, status, __ierr )
MPI_Fint *request;
MPI_Fint *status;
MPI_Fint *__ierr;
{
    MPI_Request lrequest;
    MPI_Status c_status;

    lrequest = MPI_Request_f2c(*request);
    *__ierr = MPI_Wait(&lrequest, &c_status);
#ifdef OLD_POINTER
    /* By checking for null, we handle persistant requests */
    if (lrequest == MPI_REQUEST_NULL) {
        MPIR_RmPointer( ((int)(lrequest)) );
        *request = 0;
    }
    else
#endif
        *request = MPI_Request_c2f(lrequest);

    MPI_Status_c2f(&c_status, status);
}
