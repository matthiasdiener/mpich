/* alltoall.c */
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
#define mpi_alltoall_ PMPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall
#else
#define mpi_alltoall_ pmpi_alltoall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_alltoall_ MPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall
#endif
#endif

 void mpi_alltoall_( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype      recvtype;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Alltoall(sendbuf,*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),recvbuf,*recvcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
