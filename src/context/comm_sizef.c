/* comm_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_size_ PMPI_COMM_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_size_ pmpi_comm_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_size_ pmpi_comm_size
#else
#define mpi_comm_size_ pmpi_comm_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_size_ MPI_COMM_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_size_ mpi_comm_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_size_ mpi_comm_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_size_ ANSI_ARGS(( MPI_Comm, int *, int * ));

void mpi_comm_size_ ( comm, size, __ierr )
MPI_Comm comm;
int *size;
int *__ierr;
{
    *__ierr = MPI_Comm_size(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),size);
}
