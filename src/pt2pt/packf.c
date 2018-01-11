/* pack.c */
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
#define mpi_pack_ PMPI_PACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_ pmpi_pack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_ pmpi_pack
#else
#define mpi_pack_ pmpi_pack_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pack_ MPI_PACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_ mpi_pack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_ mpi_pack
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS  8

 void mpi_pack_ ( void *unknown, ...)
{
void         *inbuf;
int*incount;
MPI_Datatype  datatype;
void         *outbuf;
int*outcount;
int          *position;
MPI_Comm      comm;
int *__ierr;
int		buflen;
va_list ap;

va_start(ap, unknown);
inbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    *__ierr = MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ONE_CHAR, 
			  "Error in MPI_SENDRECV" );
    return;
} else { if (_numargs() == NUMPARAMS+2) {
        buflen = 	va_arg(ap, int) /8;          /* This is in bits. */
	incount =       va_arg (ap, int *);
	datatype =      va_arg(ap, MPI_Datatype);
	outbuf = 	va_arg(ap, void *);
	buflen =	va_arg(ap, int ) /8;
} else {
	incount =       va_arg (ap, int *);
	datatype =      va_arg(ap, MPI_Datatype);
	outbuf =	va_arg(ap, void *);
}
}

outcount =	va_arg(ap, int *);
position =	va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Pack(MPIR_F_PTR(inbuf),*incount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),outbuf,*outcount,position,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

 void mpi_pack_ ( inbuf, incount, type, outbuf, outcount, position, comm, __ierr )
void         *inbuf;
int*incount;
MPI_Datatype  type;
void         *outbuf;
int*outcount;
int          *position;
MPI_Comm      comm;
int *__ierr;
{
_fcd temp;
if (_isfcd(inbuf)) {
	temp = _fcdtocp(inbuf);
	inbuf = (void *)temp;
}
if (_isfcd(outbuf)) {
	temp = _fcdtocp(outbuf);
	outbuf = (void *)temp;
}

*__ierr = MPI_Pack(MPIR_F_PTR(inbuf),*incount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),outbuf,*outcount,position,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_pack_ ANSI_ARGS(( void *, int *, MPI_Datatype, void *, int *, 
			   int *, MPI_Comm, int * ));
void mpi_pack_ ( inbuf, incount, type, outbuf, outcount, position, comm, 
		 __ierr )
void         *inbuf;
int*incount;
MPI_Datatype  type;
void         *outbuf;
int*outcount;
int          *position;
MPI_Comm      comm;
int *__ierr;
{
    *__ierr = MPI_Pack(MPIR_F_PTR(inbuf),*incount,
		       (MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),
		       outbuf,*outcount,position,
		       (MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
#endif
