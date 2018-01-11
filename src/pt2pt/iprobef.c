/* iprobe.c */
/* Fortran interface file for sun4 */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_iprobe_ PMPI_IPROBE
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe
#else
#define mpi_iprobe_ pmpi_iprobe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_iprobe_ MPI_IPROBE
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe
#endif
#endif

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
