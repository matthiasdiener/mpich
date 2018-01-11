/* scan.c */
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
#define mpi_scan_ PMPI_SCAN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scan_ pmpi_scan__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scan_ pmpi_scan
#else
#define mpi_scan_ pmpi_scan_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_scan_ MPI_SCAN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_scan_ mpi_scan__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_scan_ mpi_scan
#endif
#endif

 void mpi_scan_ ( sendbuf, recvbuf, count, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int*count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Scan(sendbuf,recvbuf,*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Op)MPIR_ToPointer( *(int*)(op) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
