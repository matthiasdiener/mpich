/* graph_nbr_cnt.c */
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
#define mpi_graph_neighbors_count_ PMPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count
#else
#define mpi_graph_neighbors_count_ pmpi_graph_neighbors_count_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graph_neighbors_count_ MPI_GRAPH_NEIGHBORS_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_count_ mpi_graph_neighbors_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_count_ mpi_graph_neighbors_count
#endif
#endif

 void mpi_graph_neighbors_count_ ( comm, rank, nneighbors, __ierr )
MPI_Comm  comm;
int*rank;
int      *nneighbors;
int *__ierr;
{
*__ierr = MPI_Graph_neighbors_count(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*rank,nneighbors);
}
