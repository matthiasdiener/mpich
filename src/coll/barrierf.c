/* barrier.c */
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
#define mpi_barrier_ PMPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ pmpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ pmpi_barrier
#else
#define mpi_barrier_ pmpi_barrier_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_barrier_ MPI_BARRIER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_barrier_ mpi_barrier__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_barrier_ mpi_barrier
#endif
#endif

 void mpi_barrier_ ( comm, __ierr )
MPI_Comm comm;
int *__ierr;
{
*__ierr = MPI_Barrier(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
