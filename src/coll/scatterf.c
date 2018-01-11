/* scatter.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_scatter_ PMPI_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatter_ pmpi_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatter_ pmpi_scatter
#else
#define mpi_scatter_ pmpi_scatter_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_scatter_ MPI_SCATTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scatter_ mpi_scatter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scatter_ mpi_scatter
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 9

 void mpi_scatter_ ( void *unknown, ...)
{
void            *sendbuf;
int		*sendcnt;
MPI_Datatype    *sendtype;
void            *recvbuf;
int		*recvcnt;
MPI_Datatype    *recvtype;
int		*root;
MPI_Comm        *comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_SCATTER" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcnt =     	va_arg (ap, int *);
sendtype =      va_arg(ap, MPI_Datatype *);
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcnt =     	va_arg (ap, int *);
recvtype =      va_arg(ap, MPI_Datatype *);
root =		va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm*);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf),*sendcnt,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcnt,*recvtype,*root,*comm );
}

#else

 void mpi_scatter_ ( sendbuf, sendcnt, sendtype, 
    recvbuf, recvcnt, recvtype, 
    root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype     *sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype     *recvtype;
int*root;
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

*__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf),*sendcnt,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcnt,*recvtype,*root,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
void mpi_scatter_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, 
			      void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                              MPI_Fint *, MPI_Fint * ));

void mpi_scatter_ ( sendbuf, sendcnt, sendtype, 
                    recvbuf, recvcnt, recvtype, 
                    root, comm, __ierr )
void     *sendbuf;
MPI_Fint *sendcnt;
MPI_Fint *sendtype;
void     *recvbuf;
MPI_Fint *recvcnt;
MPI_Fint *recvtype;
MPI_Fint *root;
MPI_Fint *comm;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                          MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                          (int)*recvcnt, MPI_Type_f2c(*recvtype), 
                          (int)*root, MPI_Comm_f2c(*comm));
}
#endif
