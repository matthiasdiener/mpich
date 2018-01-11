/* dims_create.c */
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

 void mpi_dims_create_(nnodes, ndims, dims, __ierr )
int*nnodes;
int*ndims;
int *dims;
int *__ierr;
{
*__ierr = MPI_Dims_create(*nnodes,*ndims,dims);
}
