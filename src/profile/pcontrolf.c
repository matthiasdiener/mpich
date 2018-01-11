/* pcontrol.c */
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
#define mpi_pcontrol_ PMPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol
#else
#define mpi_pcontrol_ pmpi_pcontrol_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pcontrol_ MPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol
#endif
#endif

 void mpi_pcontrol_( level, __ierr )
int*level;
int *__ierr;
{
*__ierr = MPI_Pcontrol(*level);
}
