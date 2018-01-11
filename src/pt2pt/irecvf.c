/* irecv.c */
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
#define mpi_irecv_ PMPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irecv_ pmpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irecv_ pmpi_irecv
#else
#define mpi_irecv_ pmpi_irecv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_irecv_ MPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irecv_ mpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irecv_ mpi_irecv
#endif
#endif

 void mpi_irecv_( buf, count, datatype, source, tag, comm, request, __ierr )
void             *buf;
int*count;
MPI_Datatype     datatype;
int*source;
int*tag;
MPI_Comm         comm;
MPI_Request      *request;
int *__ierr;
{
MPI_Request lrequest;
*__ierr = MPI_Irecv(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
