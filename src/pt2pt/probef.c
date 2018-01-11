/* probe.c */
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
#define mpi_probe_ PMPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ pmpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ pmpi_probe
#else
#define mpi_probe_ pmpi_probe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_probe_ MPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ mpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ mpi_probe
#endif
#endif

 void mpi_probe_( source, tag, comm, status, __ierr )
int*source;
int*tag;
MPI_Comm    comm;
MPI_Status  *status;
int *__ierr;
{
*__ierr = MPI_Probe(*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
