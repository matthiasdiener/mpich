/* bsend_init.c */
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
#define mpi_bsend_init_ PMPI_BSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_init_ pmpi_bsend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_init_ pmpi_bsend_init
#else
#define mpi_bsend_init_ pmpi_bsend_init_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_bsend_init_ MPI_BSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_init_ mpi_bsend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_init_ mpi_bsend_init
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_bsend_init_( void *unknown, ...)
{

/*  Must use variable argument list because buffer parameter may be a two
	word Fortran character descriptor.  If it is, need to extract the
	parameters by hand from the list as it is defined as a void * instead
	of _fcd * and it has already been pulled off the list. */

char *		buf;
int *		count;
MPI_Datatype	datatype;
int*		dest;
int*		tag;
MPI_Comm      	comm;
MPI_Request*	request;
int *		__ierr;
MPI_Request 	lrequest;
int buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
	buflen = (va_arg(ap, int)) / 8;		/* This is in bits. */
}
count = 	va_arg (ap, int *);
datatype = 	va_arg(ap, MPI_Datatype);
dest = 		va_arg(ap, int *);
tag =		va_arg(ap, int *);
comm =		va_arg(ap, MPI_Comm);
request =	va_arg(ap, MPI_Request *);
__ierr =	va_arg(ap, int *);


*__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}

#else
 void mpi_bsend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int*		count;
MPI_Datatype  	datatype;
int*		dest;
int*		tag;
MPI_Comm      	comm;
MPI_Request*	request;
int *		__ierr;
{
MPI_Request lrequest;
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_bsend_init_ ANSI_ARGS(( void *, int *, MPI_Datatype, int *, int *,
				 MPI_Comm, MPI_Request *, int * ));

void mpi_bsend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int*count;
MPI_Datatype  datatype;
int*dest;
int*tag;
MPI_Comm      comm;
MPI_Request   *request;
int *__ierr;
{
    MPI_Request lrequest;
    *__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),*count,
			     (MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
			     *dest,*tag,
			     (MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),
			     &lrequest);
    *(int*)request = MPIR_FromPointer(lrequest);
}
#endif
