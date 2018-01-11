/* comm_rsize.c */
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
#define mpi_comm_remote_size_ PMPI_COMM_REMOTE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_size_ pmpi_comm_remote_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_size_ pmpi_comm_remote_size
#else
#define mpi_comm_remote_size_ pmpi_comm_remote_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_remote_size_ MPI_COMM_REMOTE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_size_ mpi_comm_remote_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_size_ mpi_comm_remote_size
#endif
#endif

 void mpi_comm_remote_size_ ( comm, size, __ierr )
MPI_Comm  comm;
int      *size;
int *__ierr;
{
*__ierr = MPI_Comm_remote_size(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),size);
}
