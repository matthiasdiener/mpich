/* sendrecv.c */
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

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 13

 void mpi_sendrecv_( void * unknown, ...)
{
void         	*sendbuf;
int		*sendcount;
MPI_Datatype  	sendtype;
int		*dest,*sendtag;
void         	*recvbuf;
int		*recvcount;
MPI_Datatype  	recvtype;
int		*source,*recvtag;
MPI_Comm      	comm;
MPI_Status   	*status;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
	printf("Either both or neither buffer may be of type character\n");
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int ) / 8;         /* The length is in bits. */
}
sendcount =         va_arg(ap, int *);
sendtype =      va_arg(ap, MPI_Datatype);
dest =          va_arg(ap, int *);
sendtag =           va_arg(ap, int *);
recvbuf =		va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) / 8;         /* The length is in bits. */
}
recvcount =         va_arg(ap, int *);
recvtype =      va_arg(ap, MPI_Datatype);
source =          va_arg(ap, int *);
recvtag =          va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
status =        va_arg(ap, MPI_Status *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),*dest,*sendtag,
         MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#else

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
_fcd temp;
if (_isfcd(sendbuf)) {
	temp = _fcdtocp(sendbuf);
	sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
	temp = _fcdtocp(recvbuf);
	recvbuf = (void *)temp;
}
*__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),*dest,*sendtag,
         MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#endif
#else
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
*__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf),*sendcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),*dest,*sendtag,
         MPIR_F_PTR(recvbuf),*recvcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
#endif
