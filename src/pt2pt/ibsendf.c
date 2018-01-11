/* ibsend.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_ibsend_ PMPI_IBSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ibsend_ pmpi_ibsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ibsend_ pmpi_ibsend
#else
#define mpi_ibsend_ pmpi_ibsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_ibsend_ MPI_IBSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ibsend_ mpi_ibsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ibsend_ mpi_ibsend
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS  8

 void mpi_ibsend_( void *unknown, ...)
{
void             *buf;
int		*count;
MPI_Datatype     *datatype;
int		*dest;
int		*tag;
MPI_Comm         *comm;
MPI_Request      *request;
int 		*__ierr;
int		buflen;
va_list ap;
MPI_Request lrequest;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =         va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype *);
dest =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm*);
request =       va_arg(ap, MPI_Request *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Ibsend(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,
		     *comm,&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}

#else

 void mpi_ibsend_( buf, count, datatype, dest, tag, comm, request, __ierr )
void             *buf;
int*count;
MPI_Datatype     *datatype;
int*dest;
int*tag;
MPI_Comm         *comm;
MPI_Request      *request;
int *__ierr;
{
MPI_Request lrequest;
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Ibsend(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,
		     *comm,&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_ibsend_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                             MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                             MPI_Fint * ));

void mpi_ibsend_( buf, count, datatype, dest, tag, comm, request, __ierr )
void     *buf;
MPI_Fint *count;
MPI_Fint *datatype;
MPI_Fint *dest;
MPI_Fint *tag;
MPI_Fint *comm;
MPI_Fint *request;
MPI_Fint *__ierr;
{
    MPI_Request lrequest;
    *__ierr = MPI_Ibsend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                         (int)*dest,(int)*tag,MPI_Comm_f2c(*comm),
                         &lrequest);
    *request = MPI_Request_c2f(lrequest);
}
#endif
