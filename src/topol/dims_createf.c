/* dims_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_dims_create_ PMPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create
#else
#define mpi_dims_create_ pmpi_dims_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_dims_create_ MPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_dims_create_ ANSI_ARGS(( int *, int *, int *, int * ));

void mpi_dims_create_(nnodes, ndims, dims, __ierr )
int*nnodes;
int*ndims;
int *dims;
int *__ierr;
{
    *__ierr = MPI_Dims_create(*nnodes,*ndims,dims);
}
