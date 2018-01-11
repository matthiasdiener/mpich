/* finalize.c */
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
#define mpi_finalize_ PMPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ pmpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ pmpi_finalize
#else
#define mpi_finalize_ pmpi_finalize_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_finalize_ MPI_FINALIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalize_ mpi_finalize__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalize_ mpi_finalize
#endif
#endif

 void mpi_finalize_(__ierr )
int *__ierr;
{
*__ierr = MPI_Finalize();
}
