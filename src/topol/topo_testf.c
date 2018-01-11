/* topo_test.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_topo_test_ PMPI_TOPO_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_topo_test_ pmpi_topo_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_topo_test_ pmpi_topo_test
#else
#define mpi_topo_test_ pmpi_topo_test_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_topo_test_ MPI_TOPO_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_topo_test_ mpi_topo_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_topo_test_ mpi_topo_test
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_topo_test_ ANSI_ARGS(( MPI_Comm, int *, int * ));

void mpi_topo_test_ ( comm, top_type, __ierr )
MPI_Comm  comm;
int      *top_type; 
int *__ierr;
{
    *__ierr = MPI_Topo_test(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),top_type);
}
