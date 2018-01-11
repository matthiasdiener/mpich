/* errget.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_errhandler_get_ PMPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get
#else
#define mpi_errhandler_get_ pmpi_errhandler_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_get_ MPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_errhandler_get_ ANSI_ARGS(( MPI_Comm, MPI_Errhandler *, int * ));

void mpi_errhandler_get_( comm, errhandler, __ierr )
MPI_Comm comm;
MPI_Errhandler *errhandler;
int *__ierr;
{
    MPI_Errhandler new;
    *__ierr = MPI_Errhandler_get(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ), &new );
    *(int*)errhandler = MPIR_FromPointer( new );
}
