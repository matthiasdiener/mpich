/* graph_nbr.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

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
void mpi_graph_neighbors_ ANSI_ARGS(( MPI_Fint*, MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint *, MPI_Fint * ));

void mpi_graph_neighbors_ ( comm, rank, maxneighbors, neighbors, __ierr )
MPI_Fint *comm;
MPI_Fint *rank;
MPI_Fint *maxneighbors;
MPI_Fint *neighbors;
MPI_Fint *__ierr;
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Graph_neighbors( MPI_Comm_f2c(*comm), *rank,
                                       *maxneighbors, neighbors);
    else {
        int *lneighbors;
        int i;

	MPIR_FALLOC(lneighbors,(int*)MALLOC(sizeof(int)* (int)*maxneighbors),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_neighbors");

        *__ierr = MPI_Graph_neighbors( MPI_Comm_f2c(*comm), (int)*rank,
                                       (int)*maxneighbors, lneighbors);
        for (i=0; i<*maxneighbors; i++)
	    neighbors[i] = (MPI_Fint)lneighbors[i];
	
	FREE( lneighbors );
    }
}
