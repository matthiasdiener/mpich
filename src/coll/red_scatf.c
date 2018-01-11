/* red_scat.c */
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
#define mpi_reduce_scatter_ PMPI_REDUCE_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_scatter_ pmpi_reduce_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_scatter_ pmpi_reduce_scatter
#else
#define mpi_reduce_scatter_ pmpi_reduce_scatter_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_reduce_scatter_ MPI_REDUCE_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_reduce_scatter_ mpi_reduce_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_reduce_scatter_ mpi_reduce_scatter
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 7

 void mpi_reduce_scatter_ ( void *unknown, ...)
{
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
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
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcnts =     	va_arg(ap, int *);
datatype =      va_arg(ap, MPI_Datatype);
op =		va_arg(ap, MPI_Op);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Reduce_scatter(MPIR_F_PTR(sendbuf),
			     MPIR_F_PTR(recvbuf),recvcnts,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

 void mpi_reduce_scatter_ ( sendbuf, recvbuf, recvcnts, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
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

*__ierr = MPI_Reduce_scatter(MPIR_F_PTR(sendbuf),
			     MPIR_F_PTR(recvbuf),recvcnts,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else

 void mpi_reduce_scatter_ ( sendbuf, recvbuf, recvcnts, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Reduce_scatter(MPIR_F_PTR(sendbuf),
			     MPIR_F_PTR(recvbuf),recvcnts,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
#endif
