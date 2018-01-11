/* graphdims_get.c */
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
#define mpi_graphdims_get_ PMPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get
#else
#define mpi_graphdims_get_ pmpi_graphdims_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graphdims_get_ MPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get
#endif
#endif

 void mpi_graphdims_get_ ( comm, nnodes, nedges, __ierr )
MPI_Comm  comm;
int              *nnodes;
int              *nedges;
int *__ierr;
{
*__ierr = MPI_Graphdims_get(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),nnodes,nedges);
}
