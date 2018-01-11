/* graph_map.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

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
void mpi_graph_map_ ANSI_ARGS(( MPI_Comm, int *, int *, int *, int *, int * ));

void mpi_graph_map_ ( comm_old, nnodes, index, edges, newrank, __ierr )
MPI_Comm comm_old;
int*nnodes;
int     *index;
int     *edges;
int     *newrank;
int *__ierr;
{
    *__ierr = MPI_Graph_map(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm_old) ),
	*nnodes,index,edges,newrank);
}
