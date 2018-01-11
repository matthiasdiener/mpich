/* isend.c */
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
#define mpi_isend_ PMPI_ISEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_isend_ pmpi_isend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_isend_ pmpi_isend
#else
#define mpi_isend_ pmpi_isend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_isend_ MPI_ISEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_isend_ mpi_isend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_isend_ mpi_isend
#endif
#endif

 void mpi_isend_( buf, count, datatype, dest, tag, comm, request, __ierr )
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
*__ierr = MPI_Isend(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
