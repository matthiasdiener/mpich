/* wait.c */
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

 void mpi_wait_ ( request, status, __ierr )
MPI_Request  *request;
MPI_Status   *status;
int *__ierr;
{
MPI_Request lrequest;
lrequest = (MPI_Request)MPIR_ToPointer(*(int*)request);
*__ierr = MPI_Wait(&lrequest,status);
/* By checking for null, we handle persistant requests */
if (lrequest == MPI_REQUEST_NULL) {
    MPIR_RmPointer( *((int *)(request)) );
    *(int *)request = 0;
    }
}
