/* allgather.c */
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
#define mpi_allgather_ PMPI_ALLGATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgather_ pmpi_allgather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgather_ pmpi_allgather
#else
#define mpi_allgather_ pmpi_allgather_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_allgather_ MPI_ALLGATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgather_ mpi_allgather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgather_ mpi_allgather
#endif
#endif

 void mpi_allgather_ ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int*recvcount;
MPI_Datatype      recvtype;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
