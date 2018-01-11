/* bsend.c */
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
#define mpi_bsend_ PMPI_BSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_ pmpi_bsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_ pmpi_bsend
#else
#define mpi_bsend_ pmpi_bsend_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_bsend_ MPI_BSEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bsend_ mpi_bsend__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bsend_ mpi_bsend
#endif
#endif

 void mpi_bsend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Bsend(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
