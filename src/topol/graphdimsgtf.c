/* graphdims_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_graphdims_get_ PMPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ pmpi_graphdims_get
#else
#define mpi_graphdims_get_ pmpi_graphdims_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_graphdims_get_ MPI_GRAPHDIMS_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_graphdims_get_ mpi_graphdims_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_graphdims_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                    MPI_Fint * ));

void mpi_graphdims_get_ ( comm, nnodes, nedges, __ierr )
MPI_Fint *comm;
MPI_Fint *nnodes;
MPI_Fint *nedges;
MPI_Fint *__ierr;
{
    int lnnodes;
    int lnedges;

    *__ierr = MPI_Graphdims_get( MPI_Comm_f2c(*comm), &lnnodes,
                                 &lnedges);
    *nnodes = (MPI_Fint)lnnodes;
    *nedges = (MPI_Fint)lnedges;
    
}
