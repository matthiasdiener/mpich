/* graph_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_graph_create_ PMPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_create_ pmpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_create_ pmpi_graph_create
#else
#define mpi_graph_create_ pmpi_graph_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graph_create_ MPI_GRAPH_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graph_create_ mpi_graph_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graph_create_ mpi_graph_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_graph_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

void mpi_graph_create_ ( comm_old, nnodes, index, edges, reorder, comm_graph,
			 __ierr )
MPI_Fint *comm_old;
MPI_Fint *nnodes;
MPI_Fint *index;
MPI_Fint *edges;
MPI_Fint *reorder;
MPI_Fint *comm_graph;
MPI_Fint *__ierr;
{

    MPI_Comm lcomm_graph;

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), *nnodes,
                                    index, edges,
                                    MPIR_FROM_FLOG(*reorder), 
                                    &lcomm_graph);
    else {
        int i;
        int nedges;
        int *lindex;
        int *ledges;


        MPI_Graphdims_get(MPI_Comm_f2c(*comm_old), nnodes, &nedges);	
	MPIR_FALLOC(lindex,(int*)MALLOC(sizeof(int)* (int)*nnodes),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_create");
	MPIR_FALLOC(ledges,(int*)MALLOC(sizeof(int)* (int)nedges),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_create");

        for (i=0; i<(int)*nnodes; i++)
	    lindex[i] = (int)index[i];

        for (i=0; i<nedges; i++)
	    ledges[i] = (int)edges[i];

        *__ierr = MPI_Graph_create( MPI_Comm_f2c(*comm_old), (int)*nnodes,
                                    lindex, ledges,
                                    MPIR_FROM_FLOG(*reorder), 
                                    &lcomm_graph);
	FREE( lindex );
	FREE( ledges );
    }
    *comm_graph = MPI_Comm_c2f(lcomm_graph);
}
