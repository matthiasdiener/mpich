/* ssend.c */
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
#define mpi_ssend_ PMPI_SSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ssend_ pmpi_ssend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ssend_ pmpi_ssend
#else
#define mpi_ssend_ pmpi_ssend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_ssend_ MPI_SSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ssend_ mpi_ssend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ssend_ mpi_ssend
#endif
#endif

 void mpi_ssend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Ssend(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
