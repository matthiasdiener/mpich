/* recv.c */
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
#define mpi_recv_ PMPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ pmpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ pmpi_recv
#else
#define mpi_recv_ pmpi_recv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_recv_ MPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ mpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ mpi_recv
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_recv_( void *unknown, ...)
{
void             *buf;
int*count,*source,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
int *__ierr;
int		buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =         va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype);
source =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
status =        va_arg(ap, MPI_Status *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Recv(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#else

 void mpi_recv_( buf, count, datatype, source, tag, comm, status, __ierr )
void             *buf;
int*count,*source,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Recv(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#endif
#else

void mpi_recv_( buf, count, datatype, source, tag, comm, status, __ierr )
void             *buf;
int*count,*source,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
int *__ierr;
{
*__ierr = MPI_Recv(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
#endif
