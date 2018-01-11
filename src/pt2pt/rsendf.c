/* rsend.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_rsend_ PMPI_RSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_ pmpi_rsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_ pmpi_rsend
#else
#define mpi_rsend_ pmpi_rsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_rsend_ MPI_RSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_ mpi_rsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_ mpi_rsend
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 7

 void mpi_rsend_( void *unknown, ...)
{
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
int *__ierr;
int		buflen;
va_list         ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) / 8;         /* The length is in bits. */
}
count =         va_arg(ap, int *);
datatype =      va_arg(ap, MPI_Datatype*);
dest =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Rsend(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm);
}

#else

 void mpi_rsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *) temp;
}
*__ierr = MPI_Rsend(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_rsend_ ANSI_ARGS(( void *, int *, MPI_Datatype *, int *, int *, 
			    MPI_Comm *, int * ));

void mpi_rsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int              *count,*dest,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
int *__ierr;
{
    *__ierr = MPI_Rsend(MPIR_F_PTR(buf),*count,*datatype,
			*dest,*tag,*comm);}
#endif
