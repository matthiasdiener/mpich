/* unpack.c */
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
#define mpi_unpack_ PMPI_UNPACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_unpack_ pmpi_unpack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_unpack_ pmpi_unpack
#else
#define mpi_unpack_ pmpi_unpack_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_unpack_ MPI_UNPACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_unpack_ mpi_unpack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_unpack_ mpi_unpack
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_unpack_ ( void *unknown, ...)
{
void         	*inbuf;
int		*insize;
int          	*position;
void         	*outbuf;
int		*outcount;
MPI_Datatype  	datatype;
MPI_Comm      	comm;
int 		*__ierr;
int		buflen;
va_list         ap;

va_start(ap, unknown);
inbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    *__ierr = MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ONE_CHAR, 
			  "Error in MPI_UNPACK" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) / 8;           /* The length is in bits. */
}
insize =        va_arg(ap, int *);
position =      va_arg(ap, int *);
outbuf =	va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) / 8;           /* The length is in bits. */
}
outcount =      va_arg(ap, int *);
datatype =      va_arg(ap, MPI_Datatype);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);


*__ierr = MPI_Unpack(inbuf,*insize,position,MPIR_F_PTR(outbuf),*outcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

 void mpi_unpack_ ( inbuf, insize, position, outbuf, outcount, type, comm, __ierr )
void         *inbuf;
int*insize;
int          *position;
void         *outbuf;
int*outcount;
MPI_Datatype  type;
MPI_Comm      comm;
int *__ierr;
{
_fcd temp;
if (_isfcd(inbuf)) {
	temp = _fcdtocp(inbuf);
	inbuf = (void *) temp;
}
if (_isfcd(outbuf)) {
	temp = _fcdtocp(outbuf);
	outbuf = (void *) temp;
}
*__ierr = MPI_Unpack(inbuf,*insize,position,MPIR_F_PTR(outbuf),*outcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_unpack_ ANSI_ARGS(( void *, int *, int *, void *, int *, MPI_Datatype,
			     MPI_Comm, int * ));

void mpi_unpack_ ( inbuf, insize, position, outbuf, outcount, type, comm, 
		   __ierr )
void         *inbuf;
int*insize;
int          *position;
void         *outbuf;
int*outcount;
MPI_Datatype  type;
MPI_Comm      comm;
int *__ierr;
{
    *__ierr = MPI_Unpack(inbuf,*insize,position,MPIR_F_PTR(outbuf),*outcount,
			 (MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),
			 (MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
#endif
