/* irsend.c */
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
#define mpi_irsend_ PMPI_IRSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irsend_ pmpi_irsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irsend_ pmpi_irsend
#else
#define mpi_irsend_ pmpi_irsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_irsend_ MPI_IRSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irsend_ mpi_irsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irsend_ mpi_irsend
#endif
#endif

 void mpi_irsend_( buf, count, datatype, dest, tag, comm, request, __ierr )
void             *buf;
int*count;
MPI_Datatype     datatype;
int*dest;
int*tag;
MPI_Comm         comm;
MPI_Request      *request;
int *__ierr;
{
MPI_Request lrequest;
*__ierr = MPI_Irsend(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
