/* graph_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

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
void mpi_graph_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_graph_get_ ( comm, maxindex, maxedges, index, edges, __ierr )
MPI_Fint *comm;
MPI_Fint *maxindex;
MPI_Fint *maxedges;
MPI_Fint *index; 
MPI_Fint *edges;
MPI_Fint *__ierr;
{
    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Graph_get( MPI_Comm_f2c(*comm), *maxindex,
                                 *maxedges, index, edges);
    else {
        int *lindex;
        int *ledges;
        int i;

	MPIR_FALLOC(lindex,(int*)MALLOC(sizeof(int)* (int)*maxindex),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_get");
	MPIR_FALLOC(ledges,(int*)MALLOC(sizeof(int)* (int)*maxedges),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Graph_get");

        *__ierr = MPI_Graph_get( MPI_Comm_f2c(*comm), (int)*maxindex,
                                 (int)*maxedges, lindex, ledges);
        for (i=0; i<*maxindex; i++)
	    index[i] = (MPI_Fint)lindex[i];
        for (i=0; i<*maxedges; i++)
	    edges[i] = (MPI_Fint)ledges[i];

	FREE( lindex );
	FREE( ledges );
    }

}
