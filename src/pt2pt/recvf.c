/* recv.c */
/* Fortran interface file */
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
#define mpi_recv_ PMPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ pmpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ pmpi_recv
#else
#define mpi_recv_ pmpi_recv_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_recv_ MPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ mpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ mpi_recv
#endif
#endif

 void mpi_recv_( buf, count, datatype, source, tag, comm, status, __ierr )
void             *buf;
int*count,*source,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
int *__ierr;
{
*__ierr = MPI_Recv(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),status);
}
