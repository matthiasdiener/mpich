/* reduce.c */
/* Custom Fortran interface file */
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
#define mpi_reduce_ PMPI_REDUCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_ pmpi_reduce__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_ pmpi_reduce
#else
#define mpi_reduce_ pmpi_reduce_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_reduce_ MPI_REDUCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_ mpi_reduce__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_ mpi_reduce
#endif
#endif

 void mpi_reduce_ ( sendbuf, recvbuf, count, datatype, op, root, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int*count;
MPI_Datatype      datatype;
MPI_Op            op;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Reduce(MPIR_F_PTR(sendbuf),MPIR_F_PTR(recvbuf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
