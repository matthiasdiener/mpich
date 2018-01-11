/* ssend_init.c */
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
#define mpi_ssend_init_ PMPI_SSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ssend_init_ pmpi_ssend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ssend_init_ pmpi_ssend_init
#else
#define mpi_ssend_init_ pmpi_ssend_init_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_ssend_init_ MPI_SSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_ssend_init_ mpi_ssend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_ssend_init_ mpi_ssend_init
#endif
#endif

 void mpi_ssend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
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
*__ierr = MPI_Ssend_init(buf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
