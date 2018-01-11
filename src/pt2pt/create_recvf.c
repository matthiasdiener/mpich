/* create_recv.c */
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
#define mpi_recv_init_ PMPI_RECV_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_init_ pmpi_recv_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_init_ pmpi_recv_init
#else
#define mpi_recv_init_ pmpi_recv_init_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_recv_init_ MPI_RECV_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_init_ mpi_recv_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_init_ mpi_recv_init
#endif
#endif

 void mpi_recv_init_( buf, count, datatype, source, tag, comm, request, __ierr )
void         *buf;
int*count;
MPI_Request  *request;
MPI_Datatype datatype;
int*source;
int*tag;
MPI_Comm     comm;
int *__ierr;
{
MPI_Request lrequest;
*__ierr = MPI_Recv_init(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*source,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer( lrequest );
}
