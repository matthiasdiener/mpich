/* graph_nbr.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_graph_neighbors_ ANSI_ARGS(( MPI_Comm*, int *, int *, int *, int * ));

void mpi_graph_neighbors_ ( comm, rank, maxneighbors, neighbors, __ierr )
MPI_Comm  *comm;
int       *rank;
int       *maxneighbors;
int       *neighbors;
int *__ierr;
{
    *__ierr = MPI_Graph_neighbors( *comm, *rank,*maxneighbors,neighbors);
}
