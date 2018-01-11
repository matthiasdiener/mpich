/* alltoall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_alltoall_ PMPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall
#else
#define mpi_alltoall_ pmpi_alltoall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_alltoall_ MPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_alltoall_( void *unknown, ...)
{
void             *sendbuf;
int		*sendcount;
MPI_Datatype     * sendtype;
void             *recvbuf;
int		*recvcount;
MPI_Datatype     * recvtype;
MPI_Comm         * comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_ALLTOALL" );
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcount =     va_arg (ap, int *);
sendtype =      va_arg(ap, MPI_Datatype *);
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcount =     va_arg (ap, int *);
recvtype =      va_arg(ap, MPI_Datatype *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcount,*recvtype,*comm);
}

#else

 void mpi_alltoall_( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype     *sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype     *recvtype;
MPI_Comm         *comm;
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

*__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcnt,*recvtype,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_alltoall_ ANSI_ARGS(( void *, int *, MPI_Datatype *, void *, int *,
			       MPI_Datatype *, MPI_Comm *, int * ));

void mpi_alltoall_( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype     *sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype     *recvtype;
MPI_Comm         * comm;
int *__ierr;
{
    *__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
			   MPIR_F_PTR(recvbuf),*recvcnt,*recvtype,*comm );
}
#endif
