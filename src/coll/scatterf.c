/* scatter.c */
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
#define mpi_scatter_ PMPI_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatter_ pmpi_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatter_ pmpi_scatter
#else
#define mpi_scatter_ pmpi_scatter_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_scatter_ MPI_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatter_ mpi_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatter_ mpi_scatter
#endif
#endif

 void mpi_scatter_ ( sendbuf, sendcnt, sendtype, 
    recvbuf, recvcnt, recvtype, 
    root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Scatter(sendbuf,*sendcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),recvbuf,*recvcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
