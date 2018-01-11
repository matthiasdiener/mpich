/* start.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_start_ PMPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ pmpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ pmpi_start
#else
#define mpi_start_ pmpi_start_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_start_ MPI_START
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_start_ mpi_start__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_start_ mpi_start
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_start_ ANSI_ARGS(( MPI_Request *, int * ));

void mpi_start_( request, __ierr )
MPI_Request *request;
int *__ierr;
{
    MPI_Request lrequest = (MPI_Request)MPIR_ToPointer(*(int*)request );
    *__ierr = MPI_Start( &lrequest );
}
