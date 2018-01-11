/* send.c */
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
#define mpi_send_ PMPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_ pmpi_send__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_ pmpi_send
#else
#define mpi_send_ pmpi_send_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_send_ MPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_ mpi_send__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_ mpi_send
#endif
#endif

 void mpi_send_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Send(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
