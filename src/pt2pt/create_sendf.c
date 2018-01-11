/* create_send.c */
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
#define mpi_send_init_ PMPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_init_ pmpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_init_ pmpi_send_init
#else
#define mpi_send_init_ pmpi_send_init_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_send_init_ MPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_init_ mpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_init_ mpi_send_init
#endif
#endif

 void mpi_send_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int*count;
MPI_Datatype  datatype;
int*dest;
int*tag;
MPI_Comm      comm;
MPI_Request   *request;
int *__ierr;
{
MPI_Request lrequest;
*__ierr = MPI_Send_init(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer( lrequest );
}
