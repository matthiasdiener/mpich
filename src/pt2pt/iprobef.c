/* iprobe.c */
/* Custom Fortran interface file  */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_iprobe_ PMPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe
#else
#define mpi_iprobe_ pmpi_iprobe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_iprobe_ MPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_iprobe_ ANSI_ARGS(( int *, int *, MPI_Comm, int *, MPI_Status *,
			     int * ));

void mpi_iprobe_( source, tag, comm, flag, status, __ierr )
int*source;
int*tag;
int         *flag;
MPI_Comm    comm;
MPI_Status  *status;
int *__ierr;
{
    *__ierr = MPI_Iprobe(*source,*tag,
			 (MPI_Comm)MPIR_ToPointer(*((int*)comm)),flag,status);
    *flag = MPIR_TO_FLOG(*flag);
}
