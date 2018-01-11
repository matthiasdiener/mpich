/* graph_nbr.c */
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
#define mpi_graph_neighbors_ PMPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_ pmpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_ pmpi_graph_neighbors
#else
#define mpi_graph_neighbors_ pmpi_graph_neighbors_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graph_neighbors_ MPI_GRAPH_NEIGHBORS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_neighbors_ mpi_graph_neighbors__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_neighbors_ mpi_graph_neighbors
#endif
#endif

 void mpi_graph_neighbors_ ( comm, rank, maxneighbors, neighbors, __ierr )
MPI_Comm  comm;
int*rank;
int      *maxneighbors;
int      *neighbors;
int *__ierr;
{
*__ierr = MPI_Graph_neighbors(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*rank,*maxneighbors,neighbors);
}
