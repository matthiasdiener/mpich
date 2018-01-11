/* errset.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_errhandler_set_ PMPI_ERRHANDLER_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_set_ pmpi_errhandler_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_set_ pmpi_errhandler_set
#else
#define mpi_errhandler_set_ pmpi_errhandler_set_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_set_ MPI_ERRHANDLER_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_set_ mpi_errhandler_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_set_ mpi_errhandler_set
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_errhandler_set_ ANSI_ARGS(( MPI_Comm, MPI_Errhandler *, int * ));

void mpi_errhandler_set_( comm, errhandler, __ierr )
MPI_Comm       comm;
MPI_Errhandler*errhandler;
int *__ierr;
{
    *__ierr = MPI_Errhandler_set(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),
			 (MPI_Errhandler)MPIR_ToPointer(*(int*)errhandler));
}
