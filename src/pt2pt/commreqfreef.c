/* commreq_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_request_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_request_free_( request, __ierr )
MPI_Fint *request;
MPI_Fint *__ierr;
{
    MPI_Request lrequest = MPI_Request_f2c(*request);
    *__ierr = MPI_Request_free( &lrequest );

#ifdef OLD_POINTER
/* 
   We actually need to remove the pointer from the mapping if the ref
   count is zero.  We do that by checking to see if lrequest was set to
   NULL.
 */
    if (!lrequest) {
	MPIR_RmPointer( *(int*)request );
	*(int*)request = 0;
    }
#endif
    *request = MPI_Request_c2f(lrequest);

}

