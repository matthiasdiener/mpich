/* gatherv.c */
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
#define mpi_gatherv_ PMPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv
#else
#define mpi_gatherv_ pmpi_gatherv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_gatherv_ MPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 10

 void mpi_gatherv_ ( void *unknown, ...)
{
void             *sendbuf;
int		*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int		*root;
MPI_Comm          comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ONE_CHAR, 
			 "Error in MPI_GATHERV" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcnt =     	va_arg(ap, int *);
sendtype =      va_arg(ap, MPI_Datatype);
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcnts =     	va_arg(ap, int *);
displs = 	va_arg(ap, int *);
recvtype =      va_arg(ap, MPI_Datatype);
root =		va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,displs,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#else

 void mpi_gatherv_ ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
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

*__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
        MPIR_F_PTR(recvbuf),recvcnts,displs,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */

void mpi_gatherv_ ANSI_ARGS(( void *, int *, MPI_Datatype, 
			      void *, int *, int *, MPI_Datatype, 
			      int *, MPI_Comm, int * ));

void mpi_gatherv_ ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype      recvtype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
    *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,
			  (MPI_Datatype)MPIR_ToPointer( *(int*)(sendtype) ),
			  MPIR_F_PTR(recvbuf),recvcnts,displs,
			  (MPI_Datatype)MPIR_ToPointer( *(int*)(recvtype) ),
			  *root,
			  (MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
#endif
