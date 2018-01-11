/* bsend.c */
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
#define mpi_bsend_ PMPI_BSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_ pmpi_bsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_ pmpi_bsend
#else
#define mpi_bsend_ pmpi_bsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_bsend_ MPI_BSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_ mpi_bsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_ mpi_bsend
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 7

 void mpi_bsend_( void *unknown, ...)
{
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
int		buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = (va_arg(ap, int)) / 8;          /* This is in bits. */
}
count =         va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype);
dest =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Bsend(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

 void mpi_bsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
        temp = _fcdtocp(buf);
        buf = (void *)temp;
}
*__ierr = MPI_Bsend(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_bsend_ ANSI_ARGS(( void *, int *, MPI_Datatype, int *, int *, 
			    MPI_Comm, int * ));

void mpi_bsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Bsend(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
#endif
