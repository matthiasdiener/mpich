/* gatherv.c */
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
#define mpi_gatherv_ PMPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv
#else
#define mpi_gatherv_ pmpi_gatherv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_gatherv_ MPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv
#endif
#endif

 void mpi_gatherv_ ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,displs,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
