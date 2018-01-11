/* comm_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_free_ PMPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free
#else
#define mpi_comm_free_ pmpi_comm_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_free_ MPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_free_ ANSI_ARGS(( MPI_Comm *, int * ));

void mpi_comm_free_ ( comm, __ierr )
MPI_Comm *comm;
int *__ierr;
{
    MPI_Comm lcomm = (MPI_Comm)MPIR_ToPointer( *(int*)comm );
    *__ierr = MPI_Comm_free( &lcomm );
    if (!lcomm) {
	MPIR_RmPointer( *(int*)(comm) );
	*(int*)comm = 0;
    }
}
