/* sendrecv.c */
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
#define mpi_sendrecv_ PMPI_SENDRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_ pmpi_sendrecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_ pmpi_sendrecv
#else
#define mpi_sendrecv_ pmpi_sendrecv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_sendrecv_ MPI_SENDRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_ mpi_sendrecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_ mpi_sendrecv
#endif
#endif

 void mpi_sendrecv_( sendbuf, sendcount, sendtype, dest, sendtag, 
                  recvbuf, recvcount, recvtype, source, recvtag, 
                  comm, status, __ierr )
void         *sendbuf;
int*sendcount;
MPI_Datatype  sendtype;
int*dest,*sendtag;
void         *recvbuf;
int*recvcount;
MPI_Datatype  recvtype;
int*source,*recvtag;
MPI_Comm      comm;
MPI_Status   *status;
int *__ierr;
{
*__ierr = MPI_Sendrecv(sendbuf,*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),*dest,*sendtag,recvbuf,*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
