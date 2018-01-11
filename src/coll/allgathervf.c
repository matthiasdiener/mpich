/* allgatherv.c */
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
#define mpi_allgatherv_ PMPI_ALLGATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgatherv_ pmpi_allgatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgatherv_ pmpi_allgatherv
#else
#define mpi_allgatherv_ pmpi_allgatherv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_allgatherv_ MPI_ALLGATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgatherv_ mpi_allgatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgatherv_ mpi_allgatherv
#endif
#endif

 void mpi_allgatherv_ ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcounts, displs,   recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcounts;
int              *displs;
MPI_Datatype      recvtype;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Allgatherv(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcounts,displs,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
