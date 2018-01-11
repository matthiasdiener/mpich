/* scatterv.c */
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
#define mpi_scatterv_ PMPI_SCATTERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatterv_ pmpi_scatterv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatterv_ pmpi_scatterv
#else
#define mpi_scatterv_ pmpi_scatterv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_scatterv_ MPI_SCATTERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatterv_ mpi_scatterv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatterv_ mpi_scatterv
#endif
#endif

 void mpi_scatterv_ ( sendbuf, sendcnts, displs, sendtype, 
                   recvbuf, recvcnt,  recvtype, 
                   root, comm, __ierr )
void             *sendbuf;
int              *sendcnts;
int              *displs;
MPI_Datatype      sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Scatterv(MPIR_F_PTR(sendbuf),sendcnts,displs,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),*recvcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
