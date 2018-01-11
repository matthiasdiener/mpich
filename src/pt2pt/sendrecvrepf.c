/* sendrecv_rep.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_sendrecv_replace_ PMPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace
#else
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_sendrecv_replace_ MPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 10

void mpi_sendrecv_replace_( void *unknown, ...)
{
void         	*buf;
int		*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  	datatype;
MPI_Comm      	comm;
MPI_Status   	*status;
int 		*__ierr;
va_list		ap;
int		buflen;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
	buflen = va_arg(ap, int) / 8;
}
count =         va_arg(ap, int *);
datatype =      va_arg(ap, MPI_Datatype);
dest =          va_arg(ap, int *);
sendtag =       va_arg(ap, int *);
source =        va_arg(ap, int *);
recvtag =       va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
status =        va_arg(ap, MPI_Status *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*sendtag,*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#else

void mpi_sendrecv_replace_( buf, count, datatype, dest, sendtag, 
     source, recvtag, comm, status, __ierr )
void         *buf;
int*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPI_Status   *status;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *) temp;
}
*__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*sendtag,*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_sendrecv_replace_ ANSI_ARGS(( void *, int *, MPI_Datatype, int *,
				       int *, int *, int *, MPI_Comm, 
				       MPI_Status *, int * ));
void mpi_sendrecv_replace_( buf, count, datatype, dest, sendtag, 
     source, recvtag, comm, status, __ierr )
void         *buf;
int*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPI_Status   *status;
int *__ierr;
{
    *__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,
			     (MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
				   *dest,*sendtag,*source,*recvtag,
				   (MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),
				   status);
}
#endif
