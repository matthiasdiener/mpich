/* graph_map.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_graph_map_ PMPI_GRAPH_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_map_ pmpi_graph_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_map_ pmpi_graph_map
#else
#define mpi_graph_map_ pmpi_graph_map_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graph_map_ MPI_GRAPH_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_map_ mpi_graph_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_map_ mpi_graph_map
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_graph_map_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_graph_map_ ( comm_old, nnodes, index, edges, newrank, __ierr )
MPI_Fint *comm_old;
MPI_Fint *nnodes;
MPI_Fint *index;
MPI_Fint *edges;
MPI_Fint *newrank;
MPI_Fint *__ierr;
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Graph_map( MPI_Comm_f2c(*comm_old), *nnodes,
                                 index, edges, newrank);
    else {
        int i;
        int *lindex;
        int *ledges;
        int lnewrank;
	int nedges;

        MPI_Graphdims_get(MPI_Comm_f2c(*comm_old), nnodes, &nedges);
	MPIR_FALLOC(lindex,(int*)MALLOC(sizeof(int)* (int)*nnodes),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_map");
	MPIR_FALLOC(ledges,(int*)MALLOC(sizeof(int)* (int)nedges),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_map");

        for (i=0; i<(int)*nnodes; i++)
	    lindex[i] = (int)index[i];

        for (i=0; i<nedges; i++)
	    ledges[i] = (int)edges[i];

        *__ierr = MPI_Graph_map( MPI_Comm_f2c(*comm_old), (int)*nnodes,
                                 lindex, ledges, &lnewrank);
        *newrank = (MPI_Fint)lnewrank;

	FREE( lindex );
	FREE( ledges );
    }
}
