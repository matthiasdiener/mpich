/* initialize.c */
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
#define mpi_initialized_ PMPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ pmpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ pmpi_initialized
#else
#define mpi_initialized_ pmpi_initialized_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_initialized_ MPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ mpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ mpi_initialized
#endif
#endif

 void mpi_initialized_( flag, __ierr )
int  *flag;
int *__ierr;
{
int lflag;
*__ierr = MPI_Initialized(&lflag);
*flag = MPIR_TO_FLOG(lflag);
}
