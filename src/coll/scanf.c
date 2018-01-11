/* scan.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_scan_ PMPI_SCAN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scan_ pmpi_scan__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scan_ pmpi_scan
#else
#define mpi_scan_ pmpi_scan_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_scan_ MPI_SCAN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scan_ mpi_scan__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scan_ mpi_scan
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 7

 void mpi_scan_ ( void *unknown, ...)
{
void             *sendbuf;
void             *recvbuf;
int		*count;
MPI_Datatype     *datatype;
MPI_Op            *op;
MPI_Comm          *comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_SCANF" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =     	va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype*);
op =		va_arg(ap, MPI_Op *);
comm =          va_arg(ap, MPI_Comm*);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Scan(MPIR_F_PTR(sendbuf),MPIR_F_PTR(recvbuf),*count,*datatype,
		   *op,*comm);
}

#else

 void mpi_scan_ ( sendbuf, recvbuf, count, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int*count;
MPI_Datatype     *datatype;
MPI_Op            *op;
MPI_Comm          *comm;
int *__ierr;
{
_fcd            temp;
if (_isfcd(sendbuf)) {
        temp = _fcdtocp(sendbuf);
        sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
        temp = _fcdtocp(recvbuf);
        recvbuf = (void *)temp;
}

*__ierr = MPI_Scan(MPIR_F_PTR(sendbuf),MPIR_F_PTR(recvbuf),*count,*datatype,
		   *op,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_scan_ ANSI_ARGS(( void *, void *, int *, MPI_Datatype *, MPI_Op *, 
			   MPI_Comm *, int * ));
void mpi_scan_ ( sendbuf, recvbuf, count, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int*count;
MPI_Datatype      *datatype;
MPI_Op            *op;
MPI_Comm          *comm;
int *__ierr;
{
    *__ierr = MPI_Scan(MPIR_F_PTR(sendbuf),MPIR_F_PTR(recvbuf),*count,
		       *datatype,*op, *comm);
}
#endif
