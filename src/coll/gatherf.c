/* gather.c */
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
#define mpi_gather_ PMPI_GATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gather_ pmpi_gather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gather_ pmpi_gather
#else
#define mpi_gather_ pmpi_gather_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_gather_ MPI_GATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gather_ mpi_gather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gather_ mpi_gather
#endif
#endif

 void mpi_gather_ ( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, 
   root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int*recvcount;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Gather(MPIR_F_PTR(sendbuf),*sendcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
