/* allgather.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_allgather_ PMPI_ALLGATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgather_ pmpi_allgather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgather_ pmpi_allgather
#else
#define mpi_allgather_ pmpi_allgather_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_allgather_ MPI_ALLGATHER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_allgather_ mpi_allgather__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_allgather_ mpi_allgather
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_allgather_ ( void *unknown, ...)
{
void            *sendbuf;
int		*sendcount;
MPI_Datatype    *sendtype;
void            *recvbuf;
int		*recvcount;
MPI_Datatype    *recvtype;
MPI_Comm        *comm;
int 		*__ierr;
int             buflen;
va_list 	ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_ALLGATHER" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcount =     va_arg (ap, int *);
sendtype =      va_arg(ap, MPI_Datatype *);
recvbuf =	va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcount =     va_arg (ap, int *);
recvtype =      va_arg(ap, MPI_Datatype *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcount,*recvtype,*comm);
}

#else

 void mpi_allgather_ ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype      *sendtype;
void             *recvbuf;
int*recvcount;
MPI_Datatype      *recvtype;
MPI_Comm          *comm;
int *__ierr;
{
_fcd		temp;
if (_isfcd(sendbuf)) {
	temp = _fcdtocp(sendbuf);
	sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
	temp = _fcdtocp(recvbuf);
	recvbuf = (void *)temp;
}
*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcount,*recvtype,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */

void mpi_allgather_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, void *, 
                                MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				MPI_Fint * ));

void mpi_allgather_ ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm, __ierr )
void     *sendbuf;
MPI_Fint *sendcount;
MPI_Fint *sendtype;
void     *recvbuf;
MPI_Fint *recvcount;
MPI_Fint *recvtype;
MPI_Fint *comm;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf), (int)*sendcount,
                            MPI_Type_f2c(*sendtype),
			    MPIR_F_PTR(recvbuf),
                            (int)*recvcount,
                            MPI_Type_f2c(*recvtype),
                            MPI_Comm_f2c(*comm));
}
#endif
