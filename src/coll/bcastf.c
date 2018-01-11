/* bcast.c */
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
#define mpi_bcast_ PMPI_BCAST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bcast_ pmpi_bcast__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bcast_ pmpi_bcast
#else
#define mpi_bcast_ pmpi_bcast_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_bcast_ MPI_BCAST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bcast_ mpi_bcast__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bcast_ mpi_bcast
#endif
#endif

 void mpi_bcast_ ( buffer, count, datatype, root, comm, __ierr )
void             *buffer;
int*count;
MPI_Datatype      datatype;
int*root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Bcast(MPIR_F_PTR(buffer),*count,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),*root,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
