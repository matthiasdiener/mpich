/* rsend.c */
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
#define mpi_rsend_ PMPI_RSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_ pmpi_rsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_ pmpi_rsend
#else
#define mpi_rsend_ pmpi_rsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_rsend_ MPI_RSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_ mpi_rsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_ mpi_rsend
#endif
#endif

 void mpi_rsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Rsend(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
