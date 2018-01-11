/* bufattach.c */
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
#define mpi_buffer_attach_ PMPI_BUFFER_ATTACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_attach_ pmpi_buffer_attach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_attach_ pmpi_buffer_attach
#else
#define mpi_buffer_attach_ pmpi_buffer_attach_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_buffer_attach_ MPI_BUFFER_ATTACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_attach_ mpi_buffer_attach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_attach_ mpi_buffer_attach
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS  3

 void mpi_buffer_attach_( void *unknown, ...)
{
void *buffer;
int*size;
int *__ierr;
int buflen;
va_list	ap;

va_start(ap, unknown);
buffer = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
size		= va_arg(ap, int *);
__ierr		= va_arg(ap, int *);	

*__ierr = MPI_Buffer_attach(buffer,*size);
}

#else
 void mpi_buffer_attach_( buffer, size, __ierr )
void *buffer;
int*size;
int *__ierr;
{
_fcd temp;
if (_isfcd(buffer)) {
	temp = _fcdtocp(buffer);
	buffer = (void *) temp;
}
*__ierr = MPI_Buffer_attach(buffer,*size);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_buffer_attach_ ANSI_ARGS(( void *, int *, int * ));

void mpi_buffer_attach_( buffer, size, __ierr )
void *buffer;
int*size;
int *__ierr;
{
    *__ierr = MPI_Buffer_attach(buffer,*size);
}
#endif
