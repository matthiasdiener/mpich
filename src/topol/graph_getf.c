/* graph_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_graph_get_ PMPI_GRAPH_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_get_ pmpi_graph_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_get_ pmpi_graph_get
#else
#define mpi_graph_get_ pmpi_graph_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graph_get_ MPI_GRAPH_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_get_ mpi_graph_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_get_ mpi_graph_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_graph_get_ ANSI_ARGS(( MPI_Comm *, int *, int *, int *, int *, 
				int * ));

void mpi_graph_get_ ( comm, maxindex, maxedges, index, edges, __ierr )
MPI_Comm *comm;
int *maxindex,*maxedges;
int *index, *edges;
int *__ierr;
{
    *__ierr = MPI_Graph_get( *comm, *maxindex,*maxedges,index,edges);
}
