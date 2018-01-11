/* comm_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_rank_ PMPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank
#else
#define mpi_comm_rank_ pmpi_comm_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_rank_ MPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_rank_ ANSI_ARGS(( MPI_Comm, int *, int * ));

void mpi_comm_rank_ ( comm, rank, __ierr )
MPI_Comm  comm;
int      *rank;
int *__ierr;
{
    *__ierr = MPI_Comm_rank(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),rank);
}
