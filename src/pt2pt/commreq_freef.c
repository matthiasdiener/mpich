/* commreq_free.c */
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
#define mpi_request_free_ PMPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_request_free_ pmpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_request_free_ pmpi_request_free
#else
#define mpi_request_free_ pmpi_request_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_request_free_ MPI_REQUEST_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_request_free_ mpi_request_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_request_free_ mpi_request_free
#endif
#endif

void mpi_request_free_( request, __ierr )
MPI_Request *request;
int *__ierr;
{
MPI_Request lrequest = (MPI_Request) MPIR_ToPointer(*(int*)request);
*__ierr = MPI_Request_free( &lrequest );
*(int*)request = 0;

/* 
   We actually need to remove the pointer from the mapping if the ref
   count is zero... 
 */
}
