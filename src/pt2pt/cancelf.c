/* cancel.c */
/* Fortran interface file */
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
#define mpi_cancel_ PMPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ pmpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ pmpi_cancel
#else
#define mpi_cancel_ pmpi_cancel_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cancel_ MPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ mpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ mpi_cancel
#endif
#endif

 void mpi_cancel_( request, __ierr )
MPI_Request *request;
int *__ierr;
{
*__ierr = MPI_Cancel(request);
}
