/* graph_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

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
void mpi_graph_create_ ANSI_ARGS(( MPI_Comm, int *, int *, int *, int *,
				   MPI_Comm *, int * ));

void mpi_graph_create_ ( comm_old, nnodes, index, edges, reorder, comm_graph,
			 __ierr )
MPI_Comm  comm_old;
int      *nnodes;
int      *index;
int      *edges;
int      *reorder;
MPI_Comm *comm_graph;
int      *__ierr;
{
    MPI_Comm lcomm_graph;
    *__ierr = MPI_Graph_create(
	(MPI_Comm)MPIR_ToPointer(*((int*)comm_old)),*nnodes,index,edges,
        MPIR_FROM_FLOG(*reorder),&lcomm_graph);
    if (*__ierr == 0)
	*(int *)comm_graph = MPIR_FromPointer( lcomm_graph );
}
