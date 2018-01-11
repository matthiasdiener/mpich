/* red_scat.c */
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
#define mpi_reduce_scatter_ PMPI_REDUCE_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_scatter_ pmpi_reduce_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_scatter_ pmpi_reduce_scatter
#else
#define mpi_reduce_scatter_ pmpi_reduce_scatter_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_reduce_scatter_ MPI_REDUCE_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_scatter_ mpi_reduce_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_scatter_ mpi_reduce_scatter
#endif
#endif

 void mpi_reduce_scatter_ ( sendbuf, recvbuf, recvcnts, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Reduce_scatter(sendbuf,recvbuf,recvcnts,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
