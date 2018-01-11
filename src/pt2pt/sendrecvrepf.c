/* sendrecv_rep.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_sendrecv_replace_ PMPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace
#else
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_sendrecv_replace_ MPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace
#endif
#endif

 void mpi_sendrecv_replace_( buf, count, datatype, dest, sendtag, 
     source, recvtag, comm, status, __ierr )
void         *buf;
int*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPI_Status   *status;
int *__ierr;
{
*__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*sendtag,*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
