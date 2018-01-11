/* graph_get.c */
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

 void mpi_graph_get_ ( comm, maxindex, maxedges, index, edges, __ierr )
MPI_Comm comm;
int*maxindex,*maxedges;
int *index, *edges;
int *__ierr;
{
*__ierr = MPI_Graph_get(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*maxindex,*maxedges,index,edges);
}
