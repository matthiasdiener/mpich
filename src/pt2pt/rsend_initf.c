/* rsend_init.c */
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
#define mpi_rsend_init_ PMPI_RSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_init_ pmpi_rsend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_init_ pmpi_rsend_init
#else
#define mpi_rsend_init_ pmpi_rsend_init_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_rsend_init_ MPI_RSEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_rsend_init_ mpi_rsend_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_rsend_init_ mpi_rsend_init
#endif
#endif

 void mpi_rsend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
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
*__ierr = MPI_Rsend_init(MPIR_F_PTR(buf),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}
