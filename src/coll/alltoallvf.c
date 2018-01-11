/* alltoallv.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

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

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 10

 void mpi_alltoallv_ ( void *unknown, ...)
{
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
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
        printf("Both parameters must be of type character or neither.\n");
        return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcnts =     	va_arg(ap, int *);
sdispls = 	va_arg(ap, int *);
sendtype =     	va_arg(ap, MPI_Datatype);
recvbuf =      	va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcnts =     	va_arg(ap, int *);
rdispls =	va_arg(ap, int *);
recvtype =      va_arg(ap, MPI_Datatype);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf),sendcnts,sdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,rdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

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
_fcd            temp;
if (_isfcd(sendbuf)) {
        temp = _fcdtocp(sendbuf);
        sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
        temp = _fcdtocp(recvbuf);
        recvbuf = (void *)temp;
}

*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf),sendcnts,sdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,rdispls,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else

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
#endif
