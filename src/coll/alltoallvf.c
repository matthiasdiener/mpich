/* alltoallv.c */
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
#define mpi_alltoallv_ PMPI_ALLTOALLV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoallv_ pmpi_alltoallv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoallv_ pmpi_alltoallv
#else
#define mpi_alltoallv_ pmpi_alltoallv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_alltoallv_ MPI_ALLTOALLV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoallv_ mpi_alltoallv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoallv_ mpi_alltoallv
#endif
#endif

 void mpi_alltoallv_ ( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm, __ierr )
void             *sendbuf;
int              *sendcnts;
int              *sdispls;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *rdispls; 
MPI_Datatype      recvtype;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf),sendcnts,sdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,rdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
