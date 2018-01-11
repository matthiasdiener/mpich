/* topo_test.c */
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

 void mpi_topo_test_ ( comm, top_type, __ierr )
MPI_Comm  comm;
int      *top_type; 
int *__ierr;
{
*__ierr = MPI_Topo_test(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),top_type);
}
