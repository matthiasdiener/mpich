/* allgather.c */
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

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_allgather_ ( void *unknown, ...)
{
void            *sendbuf;
int		*sendcount;
MPI_Datatype    sendtype;
void            *recvbuf;
int		*recvcount;
MPI_Datatype    recvtype;
MPI_Comm        comm;
int 		*__ierr;
int             buflen;
va_list 	ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
	printf("Both parameters must be of type character or neither.\n");
	return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcount =     va_arg (ap, int *);
sendtype =      va_arg(ap, MPI_Datatype);
recvbuf =	va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcount =     va_arg (ap, int *);
recvtype =      va_arg(ap, MPI_Datatype);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

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
_fcd		temp;
if (_isfcd(sendbuf)) {
	temp = _fcdtocp(sendbuf);
	sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
	temp = _fcdtocp(recvbuf);
	recvbuf = (void *)temp;
}
*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else
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
#endif
