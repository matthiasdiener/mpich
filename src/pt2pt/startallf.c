/* startall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpisys.h"

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
#define mpi_startall_ PMPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ pmpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ pmpi_startall
#else
#define mpi_startall_ pmpi_startall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_startall_ MPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ mpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ mpi_startall
#endif
#endif

 void mpi_startall_( count, array_of_requests, __ierr )
int*count;
MPI_Request array_of_requests[];
int *__ierr;
{
#ifdef POINTER_64_BITS
int i;
MPI_Request *r = (MPI_Request*)MALLOC(sizeof(MPI_Request)* *count);
for (i=0; i<*count; i++) {
    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
*__ierr = MPI_Startall(*count,r);
FREE( r );
#else
*__ierr = MPI_Startall(*count,array_of_requests);
#endif
}


