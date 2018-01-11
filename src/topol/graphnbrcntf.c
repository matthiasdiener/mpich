/* graph_nbr_cnt.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_graph_neighbors_count_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint * ));

void mpi_graph_neighbors_count_ ( comm, rank, nneighbors, __ierr )
MPI_Fint *comm;
MPI_Fint *rank;
MPI_Fint *nneighbors;
MPI_Fint *__ierr;
{
    int lnneighbors;

    *__ierr = MPI_Graph_neighbors_count(MPI_Comm_f2c(*comm), (int)*rank,
                                        &lnneighbors);
    *nneighbors = (MPI_Fint)lnneighbors;
}
