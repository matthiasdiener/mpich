/* allreduce.c */
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
#define mpi_allreduce_ PMPI_ALLREDUCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allreduce_ pmpi_allreduce__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allreduce_ pmpi_allreduce
#else
#define mpi_allreduce_ pmpi_allreduce_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_allreduce_ MPI_ALLREDUCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allreduce_ mpi_allreduce__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allreduce_ mpi_allreduce
#endif
#endif

 void mpi_allreduce_ ( sendbuf, recvbuf, count, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int*count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Allreduce(sendbuf,recvbuf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
