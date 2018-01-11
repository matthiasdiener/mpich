/* comm_dup.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_dup_ PMPI_COMM_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_dup_ pmpi_comm_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_dup_ pmpi_comm_dup
#else
#define mpi_comm_dup_ pmpi_comm_dup_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_dup_ MPI_COMM_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_dup_ mpi_comm_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_dup_ mpi_comm_dup
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_dup_ ANSI_ARGS(( MPI_Comm, MPI_Comm *, int * ));

void mpi_comm_dup_ ( comm, comm_out, __ierr )
MPI_Comm comm, *comm_out;
int *__ierr;
{
    MPI_Comm lcomm;
    *__ierr = MPI_Comm_dup(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ), &lcomm);
    *(int*)comm_out = MPIR_FromPointer(lcomm);
}
